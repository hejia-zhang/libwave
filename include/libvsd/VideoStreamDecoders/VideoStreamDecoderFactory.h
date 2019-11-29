//
// Created by hejia on 18-2-13.
//

#ifndef GOKU_VIDEOSTREAMDECODETHREADFACTORY_H
#define GOKU_VIDEOSTREAMDECODETHREADFACTORY_H

#include "VideoStreamDecoder.h"
#include "USBWebCamStreamDecoder.h"
#include "MP4VideoStreamDecoder.h"

enum VID_STREAM_TYPE {
  MP4_VIDEO = 1,
  RTSP_STREAM = 2,
  USB_WEBCAM = 3,
};
class VideoStreamDecoderFactory {
public:
  VideoStreamDecoderFactory(const AppConfig& config, Poco::Logger& logger) : m_config(config), m_logger(logger)  {
  }
  virtual ~VideoStreamDecoderFactory() {
  }

  VideoStreamDecoder::Ptr MakeStreamDecoder(VID_STREAM_TYPE type);

private:
  const AppConfig& m_config;
  Poco::Logger& m_logger;
};

#endif //GOKU_VIDEOSTREAMDECODETHREADFACTORY_H
