//
// Created by hjzh on 18-2-13.
//

#ifndef GOKU_VIDEOSTREAMDECODETHREADFACTORY_H
#define GOKU_VIDEOSTREAMDECODETHREADFACTORY_H

#include "VideoDecodeThread.h"
#include "USBWebCamStreamDecodeThread.h"

enum VID_STREAM_TYPE {
  MP4_VIDEO = 1,
  RTSP_STREAM = 2,
  USB_WEBCAM = 3,
};
class VideoStreamDecodeThreadFactory {
public:
  VideoStreamDecodeThreadFactory(const AppConfig& config, Poco::Logger& logger) : m_config(config), m_logger(logger)  {
  }
  virtual ~VideoStreamDecodeThreadFactory() {
  }

  VideoDecodeThread::Ptr MakeDecodeThread(VID_STREAM_TYPE type);

private:
  const AppConfig& m_config;
  Poco::Logger& m_logger;
};

#endif //GOKU_VIDEOSTREAMDECODETHREADFACTORY_H
