//
// Created by hejia on 18-2-13.
//

#include "libvsd/VideoStreamDecoders/MP4VideoStreamDecoder.h"
#include "yuv2bgr.h"
#include "resizeImageGPU.h"

bool MP4VideoStreamDecoder::Init() {
  bool res = true;
  m_strStreamUrl = m_config.m_szVideoStreamAddress;
  /// Try to find the hardware type according to hardware name we specified
  m_hwType = av_hwdevice_find_type_by_name(m_config.m_szHwName.c_str());
  if (AV_HWDEVICE_TYPE_NONE == m_hwType) {
    m_logger.error("MP4VideoDecoder::Init: "
                   "Can't find the hardware type matches with the name we specified: %s", m_config.m_szHwName);
    return false;
  }
  av_register_all();
  avcodec_register_all();
  avformat_network_init();
  avfilter_register_all();
  avdevice_register_all();
  if (RES_VID_OK != Connect()) {
    return false;
  }
  return res;
}

int MP4VideoStreamDecoder::hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type) {
  int err = 0;

  if ((err = av_hwdevice_ctx_create(&m_phwDevCtx, type,
                                    NULL, NULL, 0)) < 0) {
    m_logger.error("Failed to create specified HW device.");
    return err;
  }
  ctx->hw_device_ctx = av_buffer_ref(m_phwDevCtx);

  return err;
}

enum AVPixelFormat MP4VideoStreamDecoder::get_hw_format(AVCodecContext *ctx,
                                                        const enum AVPixelFormat *pix_fmts) {
  const enum AVPixelFormat *p;

  for (p = pix_fmts; *p != -1; p++) {
    if (*p == m_hwPixFmt)
      return *p;
  }

  m_logger.error("Failed to get HW surface format.");
  return AV_PIX_FMT_NONE;
}

VID_ERR MP4VideoStreamDecoder::Connect() {
  VID_ERR res = RES_VID_OK;
  do {
    /// Try to open input
    /// It is compatible with video and stream
    if (avformat_open_input(&m_pFormatCtx, m_strStreamUrl.c_str(), NULL, NULL) != 0) {
      m_logger.error(Poco::format("MP4VideoDecoder::Connect "
                                  "Can't open the video stream with this address: %s", m_strStreamUrl));
      res = RES_VID_ERR_OPEN_INPUT;
      break;
    }

    /// Try to find a stream
    if (avformat_find_stream_info(m_pFormatCtx, NULL) < 0) {
      m_logger.error(Poco::format("MP4VideoDecoder::Connect "
                                  "Can't find the stream info: %s", m_strStreamUrl));
      res = RES_VID_ERR_FIND_STREAM_INFO;
      break;
    }

    /// Try to find the video stream
    m_nVideoStreamIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &m_pDecoder, 0);
    if (m_nVideoStreamIndex < 0) {
      m_logger.error(Poco::format("MP4VideoDecoder::Connect "
                                  "Can't find the video stream: %s", m_strStreamUrl));
      res = RES_VID_ERR_FIND_VIDEO_STREAM;
      break;
    }

    /// Try to configure for gpu decode
    /// Check if hw type we specified is supported by this decoder
    for (int i = 0;; i++) {
      const AVCodecHWConfig *config = avcodec_get_hw_config(m_pDecoder, i);
      if (!config) {
        m_logger.error(Poco::format("Decoder %s does not support device type %s.",
                                    m_pDecoder->name, av_hwdevice_get_type_name(m_hwType)));
        res = RES_VID_DEV_TYPE_NOT_SUPPORT;
        break;
      }
      if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX && config->device_type == m_hwType) {
        m_hwPixFmt = config->pix_fmt;
        break;
      }
    }
    if (res != RES_VID_OK) {
      break;
    }

    m_pDecoderCtx = avcodec_alloc_context3(m_pDecoder);

    m_pVideo = m_pFormatCtx->streams[m_nVideoStreamIndex];
    avcodec_parameters_to_context(m_pDecoderCtx, m_pVideo->codecpar);
    hw_decoder_init(m_pDecoderCtx, m_hwType);

    if (avcodec_open2(m_pDecoderCtx, m_pDecoder, NULL) < 0) {
      m_logger.error(Poco::format("Can't open decoder: %s", m_pDecoder->name));
      res = RES_VID_OPEN_DECODER;
      break;
    }
  } while (false);
  /// If failed in connection, release the resources
  if (res != RES_VID_OK) {
    avformat_close_input(&m_pFormatCtx);
    if (m_pDecoderCtx) {
      avcodec_free_context(&m_pDecoderCtx);
    }
  } else {
    m_bConnected = true;
  }
  return res;
}

void MP4VideoStreamDecoder::Start(const std::function<FrameCBFunc> &cb) {
  m_frameCallback = cb;
  m_thread.start(*this);
}

void MP4VideoStreamDecoder::Exit() {
  m_stop = true;
  m_thread.join();
}

void MP4VideoStreamDecoder::run() {
  AVFrame *pYUVFrame = av_frame_alloc();
  AVFrame *pBGRFrame = av_frame_alloc();
  AVPacket packet;
  int align = 32;
  int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, m_pDecoderCtx->width, m_pDecoderCtx->height, align);
  unsigned char *buffer = (unsigned char *) (av_malloc(buffer_size * sizeof(uint8_t)));
  av_image_fill_arrays(pBGRFrame->data, pBGRFrame->linesize, buffer, AV_PIX_FMT_BGR24, m_pDecoderCtx->width,
                       m_pDecoderCtx->height, align);

  int bufsize0, bufsize1, resolution;
  cv::cuda::GpuMat reqMat, resMat, resizedMat;

  // av_read_play(m_pFormatCtx);//play RTSP
  Poco::Int64 nFrameNum = 0;
  int break_status = 0;
  bool is_first_time = true;
  while (!m_stop) {
    if (av_read_frame(m_pFormatCtx, &packet) != 0) {
      m_bConnected = false;
      break_status = 1;
      break;
    }

    if (packet.stream_index == m_nVideoStreamIndex) {//packet is video
      int got_picture = 0;
      int res = avcodec_decode_video2(m_pDecoderCtx, pYUVFrame, &got_picture, &packet);
      if (res >= 0 && got_picture) {
        ++nFrameNum;
        if (m_stop) {
          break_status = 2;
          break;
        }
        SharedIplImage pImg;
        if (m_config.m_ifResize) {
          pImg = cvCreateImage(cvSize(m_config.m_resizedWidth, m_config.m_resizedHeight), 8, 3);
        } else {
          pImg = cvCreateImage(cvSize(m_pDecoderCtx->width, m_pDecoderCtx->height), 8, 3);
        }
        if (is_first_time) {
          resolution = pYUVFrame->height * pYUVFrame->width;
          reqMat.create(pYUVFrame->height, pYUVFrame->width, CV_8UC3);
          resMat.create(pYUVFrame->height, pYUVFrame->width, CV_8UC3);
          resMat.step = pBGRFrame->linesize[0];
          if (m_config.m_ifResize) {
            resizedMat.create(m_config.m_resizedHeight, m_config.m_resizedWidth, CV_8UC3);
            resizedMat.step = m_config.m_resizedWidth * 3;
          }
          bufsize0 = pYUVFrame->height * pYUVFrame->linesize[0];
          bufsize1 = pYUVFrame->height * pYUVFrame->linesize[1] / 2;
          is_first_time = false;
        }
        cudaMemcpy(reqMat.data, pYUVFrame->data[0], bufsize0, cudaMemcpyHostToDevice);
        cudaMemcpy(reqMat.data + bufsize0, pYUVFrame->data[1], bufsize1, cudaMemcpyHostToDevice);
        cvtColor(reqMat.data, resMat.data, resolution, pYUVFrame->height, pYUVFrame->width, pYUVFrame->linesize[0]);
        if (m_config.m_ifResize) {
          resizeImageGPU(resizedMat.data,
                         resMat.data,
                         resizedMat.step,
                         resMat.step,
                         m_config.m_resizedHeight,
                         m_config.m_resizedWidth,
                         resMat.rows,
                         resMat.cols);
          cudaMemcpy(pImg->imageData,
                     resizedMat.data,
                     resizedMat.cols * resizedMat.rows * sizeof(uchar3),
                     cudaMemcpyDeviceToHost);
        } else {
          cudaMemcpy(pImg->imageData, resMat.data, resMat.cols * resMat.rows * sizeof(uchar3), cudaMemcpyDeviceToHost);
        }
        cv::Mat img = cv::cvarrToMat(pImg, true);
        ImageFrame data(img);
        data.frameIdx = nFrameNum;
        m_frameCallback(data);
      }
    }
    av_free_packet(&packet);
  }
  av_read_pause(m_pFormatCtx);
  av_frame_free(&pYUVFrame);
  av_frame_free(&pBGRFrame);
  av_free(buffer);

  avcodec_free_context(&m_pDecoderCtx);
  avformat_close_input(&m_pFormatCtx);
}
