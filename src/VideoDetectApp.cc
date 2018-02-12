//
// Created by hjzh on 18-2-6.
//

#include "VideoDetectApp.h"
#include "NamedMutexScopedLock.h"
#include "StringUtility.h"
#include "VideoStreamDecodeThread.h"
#include "ObjectDetectThread.h"

VideoDetectApp::VideoDetectApp() {

}

bool VideoDetectApp::ParseConfiguration() {
  bool res;
  if (!(res = ParseTFConfiguration())) {
    return res;
  }

  if (!(res = ParseVideoConfiguration())) {
    return res;
  }

  return res;
}

bool VideoDetectApp::GetOutputLayers(const std::string& szOutLayers, std::vector<std::string>& outputlayers) {
  /// The string format outlayers should look like
  /// "[detection_boxes:0", "detection_scores:0", "detection_classes:0", "num_detections:0]"
  std::size_t t_beginPos = 0;
  std::size_t t_openBracketPos;
  std::size_t t_closeBracketPos;
  bool res = true;
  do {
    if ((t_openBracketPos = szOutLayers.find(R"([)", t_beginPos)) == std::string::npos) {
      res = false;
      break;
    }
    if ((t_closeBracketPos = szOutLayers.find(R"(])", t_openBracketPos + 1)) == std::string::npos) {
      res = false;
      break;
    }

    if (t_openBracketPos >= t_closeBracketPos) {
      res = false;
      break;
    }

    /// Remove all spaces
    std::string strWithoutSpaceAndBrackets = StringUtility::remove_the_delimiter(
        szOutLayers.substr(t_openBracketPos + 1, t_closeBracketPos - t_openBracketPos - 1), ' ');

    outputlayers = StringUtility::split(strWithoutSpaceAndBrackets, ",");
  } while (false);
  return res;
}

bool VideoDetectApp::ParseTFConfiguration() {
  bool res = true;
  /// Now we need to get all required TF configuration parameters.
  try {
    /// It is supposed that there is only one input layer.
    m_config.m_szInputLayer = config().getString("input_layer");
    m_config.m_szLabelPath = config().getString("label_path");
    m_config.m_szGraphPath = config().getString("graph_path");

    /// Now we need to parse output layer
    std::string t_szOutLayers = config().getString("output_layer");
    if (!GetOutputLayers(t_szOutLayers, m_config.m_vecOutputLayers)) {
      logger().error("VideoDetectApp::ParseTFConfiguration: Can't get correct output layers!");
      res = false;
    }
  } catch (Poco::NotFoundException) {
    logger().error("VideoDetectApp::ParseTFConfiguration: Can't find some required parameters! Please set your TF parameters in properties file completely!");
    res = false;
  } catch (const std::exception& e) {
    logger().error("VideoDetectApp::ParseTFConfiguration: Some errors happended");
    res = false;
  }
  return res;
}

bool VideoDetectApp::ParseVideoConfiguration() {
  bool res = true;
  /// Now we need to get all required Video Processing parameters.
  try {
    m_config.m_nTolerableFrameProcessDely = config().getInt("tolerable_frame_delay");
    m_config.m_szVideoStreamAddress = config().getString("video_stream_address");
    m_config.m_szVideoStreamType = config().getString("video_stream_type");
    m_config.m_szHwName = config().getString("hw_name");
  } catch (Poco::NotFoundException) {
    logger().error("VideoDetectApp::ParseVideoConfiguration: Can't find some required parameters! "
                       "Please set your Video Process parameters in properties file completely!");
    res = false;
  } catch (const std::exception& e) {
    logger().error("VideoDetectApp::ParseVideoConfiguration: Some errors happended");
    res = false;
  }
  return res;
}

int VideoDetectApp::main(const std::vector<std::string> &args) {
  /// First we need to make sure there is only one process running
  /// in current
  std::string strInstanceName = "VideoDetectDemo";
  Poco::NamedMutex mutex(strInstanceName);
  NamedMutexScopedLock lock(mutex);

  if (!lock.tryLock()) {
    logger().error("There is one running Goku instance! Only one instance is permitted to run! Kill it first!");
    return EXIT_OK;
  }

  if (!ParseConfiguration()) {
    logger().error("Failed to parse properties file!");
    return EXIT_CONFIG;
  }

  /// Now we need to init RGB Process thread, TF Detector Thread, VideoProcessing thread
  /// First we will init TF Detector Thread
  ObjectDetectThread objectDetectThread(m_config, logger());
  if (!objectDetectThread.Init()) {
    logger().error("Failed to initialize objectDetectThread!");
    return EXIT_SOFTWARE;
  }
  objectDetectThread.Start();

  /// Then we will init VideoStreamDecodeThread
  VideoStreamDecodeThread videoStreamDecodeThread(m_config, logger());
  if (!videoStreamDecodeThread.Init()) {
    logger().error("Failed to initialize videoStreamDecodeThread!");
    return EXIT_SOFTWARE;
  }
  std::function<FrameCBFunc> cb = std::bind(&ObjectDetectThread::AddFrame, &objectDetectThread, std::placeholders::_1);
  videoStreamDecodeThread.Start(cb);

  /// wait for CTRL-C or kill
  waitForTerminationRequest();
  return EXIT_OK;
}