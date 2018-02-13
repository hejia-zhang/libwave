#include <Poco/Notification.h>
#include "ObjectDetectThread.h"

/**************************************** These functions are used in TF *******************************************/
tensorflow::Status ReadEntireFile(tensorflow::Env* env, const std::string& file_path, tensorflow::Tensor& output) {
  tensorflow::uint64 file_size = 0;
  TF_RETURN_IF_ERROR(env->GetFileSize(file_path, &file_size));

  std::string content;
  content.resize(file_size);

  std::unique_ptr<tensorflow::RandomAccessFile> file;
  TF_RETURN_IF_ERROR(env->NewRandomAccessFile(file_path, &file));

  tensorflow::StringPiece data;
  TF_RETURN_IF_ERROR(file->Read(0, file_size, &data, &(content)[0]));
  if (data.size() != file_size) {
    return tensorflow::errors::DataLoss("Truncated read of '", file_path, "' expected ", file_size, " got ", data.size());
  }
  output.scalar<std::string>()() = data.ToString();
  return tensorflow::Status::OK();
}

tensorflow::Status ReadTensorFrameImageData(const ImageFrame& image_frame,
                                            std::vector<tensorflow::Tensor>& out_tensors) {
  auto root = tensorflow::Scope::NewRootScope();

  /// Read Image file into a tensor named input
  tensorflow::Tensor input(tensorflow::DT_UINT8, tensorflow::TensorShape({1, image_frame.m_img.rows,
                                                                          image_frame.m_img.cols,
                                                                          3}));
  /// Copy image data into tensor memory
  auto input_tensor_mapped = input.tensor<uchar, 4>();

  const uchar* source_data = image_frame.m_img.data;

  for (int y = 0; y < image_frame.m_img.rows; ++y) {
    const uchar* source_row = source_data + (y * image_frame.m_img.cols * 3);
    for (int x = 0; x < image_frame.m_img.cols; ++x) {
      const uchar* source_pixel = source_row + (x * 3);
      for (int c = 0; c < 3; ++c) {
        const uchar* source_value = source_pixel + c;
        input_tensor_mapped(0, y, x, c) = *source_value;
      }
    }
  }

  out_tensors.push_back(input);
  return tensorflow::Status::OK();
}

// Given an image file path, read in the data, try to decode it as an image,
// resize it to the requested size, and then scale the values as desired.
tensorflow::Status ReadTensorFromImageFile(const std::string& image_path,
                                           std::vector<tensorflow::Tensor>& out_tensors) {
  auto root = tensorflow::Scope::NewRootScope();

  // Read image file into a tensor named input
  tensorflow::Tensor input(tensorflow::DT_STRING, tensorflow::TensorShape());
  TF_RETURN_IF_ERROR(ReadEntireFile(tensorflow::Env::Default(), image_path, input));

  // Use a placeholder to read input data
  auto file_reader = tensorflow::ops::Placeholder(root.WithOpName("input"), tensorflow::DataType::DT_STRING);

  std::vector<std::pair<std::string, tensorflow::Tensor>> inputs = { {"input", input} };

  // Now try to figure out what kind of file it is and decode it.
  const int wanted_channels = 3;
  tensorflow::Output image_reader;
  // Only consider .png and .JPEG files
  if (tensorflow::StringPiece(image_path).ends_with(".png")) {
    image_reader = tensorflow::ops::DecodePng(root.WithOpName("png_reader"), file_reader,
                                              tensorflow::ops::DecodePng::Channels(wanted_channels));
  } else {
    image_reader = tensorflow::ops::DecodeJpeg(root.WithOpName("jpeg_reader"), file_reader,
                                               tensorflow::ops::DecodeJpeg::Channels(wanted_channels));
  }

  // Now cast the image data to uint8 so we can do normal math on it.
  auto uint8_caster = tensorflow::ops::Cast(root.WithOpName("uint8_caster"), image_reader, tensorflow::DT_UINT8);

  // The convention for image ops in Tensorflow is that all images are expected
  // to be in batches, so that they're four-dimensional arrays with indices of
  // [batch, height, width, channel]. Because we only have a single image, we
  // have to add a batch dimension of 1 to the start with ExpandDims().
  auto dims_expander = tensorflow::ops::ExpandDims(root.WithOpName("dim"), uint8_caster, 0);

  tensorflow::GraphDef graph;
  TF_RETURN_IF_ERROR(root.ToGraphDef(&graph));

  std::unique_ptr<tensorflow::Session> session(tensorflow::NewSession(tensorflow::SessionOptions()));
  TF_RETURN_IF_ERROR(session->Create(graph));
  TF_RETURN_IF_ERROR(session->Run({inputs}, {"dim"}, {}, &out_tensors));
  return tensorflow::Status::OK();
}

// Reads a model graph definition from disk, and creates a session object you can use to run it.
tensorflow::Status LoadGraph(const std::string& graph_path, std::unique_ptr<tensorflow::Session>& session) {
  tensorflow::GraphDef graph_def;
  tensorflow::Status load_graph_status =
      tensorflow::ReadBinaryProto(tensorflow::Env::Default(), graph_path, &graph_def);
  if (!load_graph_status.ok()) {
    return tensorflow::errors::NotFound("Failed to load compute graph at '", graph_path, "'");
  }
  auto config = tensorflow::ConfigProto();
  auto gpuOptions = config.gpu_options();
  gpuOptions.set_allow_growth(true);
  auto seesionOptions = tensorflow::SessionOptions();
  seesionOptions.config = config;
  session.reset(tensorflow::NewSession(seesionOptions));
  tensorflow::Status session_create_status = (session->Create(graph_def));
  if (!session_create_status.ok()) {
    return session_create_status;
  }
  return tensorflow::Status::OK();
}
/****************************************************************************************************************/

int ObjectDetectThread::Detect(const ImageFrame &imgFrame) {
  std::string label_path = "/home/hjzh/PycharmProjects/tflearn/auv_data/auv_label_map.pbtxt";

  std::string input_layer = "image_tensor:0";
  std::vector<std::string> output_layer = { "detection_boxes:0", "detection_scores:0", "detection_classes:0", "num_detections:0" };

  // Load and initialize the model.
  std::unique_ptr<tensorflow::Session> session;
  std::string graph_path = "/home/hjzh/PycharmProjects/tflearn/models/transfer_to_auv/frozen_inference_graph.pb";
  LOG(INFO) << "graph_path: " << graph_path;
  tensorflow::Status load_graph_status = LoadGraph(graph_path, session);
  if (!load_graph_status.ok()) {
    LOG(ERROR) << "LoadGraph Error!!!" << load_graph_status;
    return 1;
  }

  // Get the image from disk as a float array of numbers, resized and normalized
  // to the specifications the main graph expects.
  std::vector<tensorflow::Tensor> resized_tensors;
  std::string image_path = "/home/hjzh/PycharmProjects/tflearn/images/image4.png";
  tensorflow::Status read_tensor_status = ReadTensorFromImageFile(image_path, resized_tensors);
  if (!read_tensor_status.ok()) {
    LOG(ERROR) << "Read Tensor Error!!!" << read_tensor_status;
    return 1;
  }

  const tensorflow::Tensor& resized_tensor = resized_tensors[0];

  LOG(INFO) << "image shape: " << resized_tensor.shape().DebugString() <<
            ", len: " << resized_tensors.size() << ", tensor type: " << resized_tensor.dtype();

  std::vector<tensorflow::Tensor> outputs;
  tensorflow::Status run_status = session->Run({{input_layer, resized_tensor}}, output_layer, {}, &outputs);
  if (!run_status.ok()) {
    LOG(ERROR) << "Running model failed: " << run_status;
    return 1;
  }

  int image_width = resized_tensor.dims();
  int image_height = 0;

  LOG(INFO) << "size: " << outputs.size() << ", image_width: " << image_width << ", image_height: " << image_height << std::endl;

  tensorflow::TTypes<float>::Flat scores = outputs[1].flat<float>();
  tensorflow::TTypes<float>::Flat classes = outputs[2].flat<float>();
  tensorflow::TTypes<float>::Flat num_detections = outputs[3].flat<float>();
  auto boxes = outputs[0].flat_outer_dims<float, 3>();

  LOG(INFO) << "num_detections: " << num_detections(0) << ", " << outputs[0].shape().DebugString();

  for (size_t i = 0; i < num_detections(0) && i < 20; i++) {
    if (scores(i) > 0.5) {
      LOG(INFO) << i << ", score: " << scores(i) * 100 << "%" << ", class: " << classes(i) << ", box" << i << ": " << "[ymin, xmin, ymax, xmax]: [" << boxes(0, i, 0)
                << ", " << boxes(0, i, 1) << ", " << boxes(0, i, 2) << ", " << boxes(0, i, 3) << "]";
    }
  }

  return 0;
}

/// The class is used as a notification containing frame image information
/// The notification will be added in queue first, then it will be dequeue and be detected.
class FrameNotification : public Poco::Notification {
public:
  typedef Poco::AutoPtr<FrameNotification> Ptr;
  FrameNotification(const ImageFrame& frame) : m_frame(frame) {
  }

  ImageFrame m_frame;
};

TF_ERR ObjectDetectThread::Init() {
  m_logger.information("Begin to initialize the TF Session...");
  m_logger.information(Poco::format("graph_path: %s", m_config.m_szGraphPath));
  m_logger.information(Poco::format("label map path: %s", m_config.m_szLabelPath));
  m_logger.information(Poco::format("input layer: %s", m_config.m_szInputLayer));
  std::string t_szOutputLayers;
  for (const auto& val : m_config.m_vecOutputLayers) {
    t_szOutputLayers += val;
    t_szOutputLayers += " ";
  }
  m_logger.information(Poco::format("output layers: [ %s ]", t_szOutputLayers));

  /// Now load graph!
  tensorflow::Status load_graph_status = LoadGraph(m_config.m_szGraphPath, m_session);
  if (!load_graph_status.ok()) {
    m_logger.error(Poco::format("ObjectDetectThread::Init() LoadGraph Error!!! Error Msg: %s",
                                load_graph_status.ToString()));
    return RES_TF_LOAD_GRAPH;
  }

  /// Open a new preview window
  if (m_config.m_openPrev) {
    cv::namedWindow("Preview");
    cv::startWindowThread();
  }

  return RES_TF_OK;
}

void ObjectDetectThread::run() {
  for (;;) {
    if (m_stop) {
      break;
    }
    Poco::Notification::Ptr pNf(m_notiQueue.waitDequeueNotification());
    if (!pNf) {
      break;
    }
    FrameNotification::Ptr pWorkNf = pNf.cast<FrameNotification>();
    try {
      /// Now you need to detect current image
      /// First you need to tranlate the frame image to a float array of numbers
      /// and resize nad normalize it to the specifications the main graph expects.
      std::vector<tensorflow::Tensor> resized_tensors;
      ReadTensorFrameImageData(pWorkNf->m_frame, resized_tensors);

      const tensorflow::Tensor& resized_tensor = resized_tensors[0];

      std::vector<tensorflow::Tensor> outputs;

      tensorflow::Status run_status = m_session->Run({{m_config.m_szInputLayer, resized_tensor}},
                                                     m_config.m_vecOutputLayers, {}, &outputs);
      if (!run_status.ok()) {
        m_logger.error(Poco::format("Running model failed: %s", run_status.ToString()));
      }

      int image_width = resized_tensor.dims();
      int image_height = 0;

      tensorflow::TTypes<float>::Flat scores = outputs[1].flat<float>();
      tensorflow::TTypes<float>::Flat classes = outputs[2].flat<float>();
      tensorflow::TTypes<float>::Flat num_detections = outputs[3].flat<float>();
      auto boxes = outputs[0].flat_outer_dims<float, 3>();

      for (size_t i = 0; i < num_detections(0) && i < 20; i++) {
        /// [ymin, xmin, ymax, xmax]
        int ymin = boxes(0, i, 0) * pWorkNf->m_frame.m_img.rows;
        int xmin = boxes(0, i, 1) * pWorkNf->m_frame.m_img.cols;
        int ymax = boxes(0, i, 2) * pWorkNf->m_frame.m_img.rows;
        int xmax = boxes(0, i, 3) * pWorkNf->m_frame.m_img.cols;
        if (scores(i) > 0.9) {
          m_logger.information(Poco::format("score: %d, class: %d box: [%d, %d, %d, %d]", int(scores(i) * 100), int(classes(i)),
                         ymin, xmin, ymax, xmax));

          /// Add a box on img
          /// Rect Rect_(_Tp _x, _Tp _y, _Tp _width, _Tp _height);
          cv::rectangle(pWorkNf->m_frame.m_img, cv::Rect(xmin, ymin, xmax - xmin, ymax - ymin), cv::Scalar(0, 255, 0));
          ///cv::imshow("Preview", pWorkNf->m_frame.m_img);
          break;
        }
      }
      /// show us the image
      cv::imshow("Preview", pWorkNf->m_frame.m_img);
    } catch (const std::exception& e) {
      m_logger.error("ObjectDetectThread::run: Something bad happened! %s", e.what());
    }
  }
}

/// It will be used as VideoStreamDecodeThread's callback function
void ObjectDetectThread::AddFrame(const ImageFrame& vf) {
  m_notiQueue.enqueueNotification(new FrameNotification(vf));
}

void ObjectDetectThread::Start(const std::function<ResultCBFunc> &cb) {
  m_cb = cb;
  /// First check if the thread is running
  if (m_thread.isRunning()) {
    return;
  }

  m_thread.start(*this);
}

void ObjectDetectThread::Start() {
  /// First check if the thread is running
  if (m_thread.isRunning()) {
    return;
  }

  m_thread.start(*this);
}

void ObjectDetectThread::Exit() {
  m_stop = true;
  m_notiQueue.clear();
  m_notiQueue.wakeUpAll();
  m_thread.join();
  cv::destroyWindow("Preview");
}
