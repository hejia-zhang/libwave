//
// Created by hejia on 18-2-13.
//

#ifndef VIDEOSTREAMDECODERFACTORY_H
#define VIDEOSTREAMDECODERFACTORY_H

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
  VideoStreamDecoderFactory(const AppConfig &config) : m_config(config) {
  }
  virtual ~VideoStreamDecoderFactory() {
  }

//  VideoStreamDecoder MakeStreamDecoder(VID_STREAM_TYPE type);

 private:
  const AppConfig &m_config;
};

#endif //VIDEOSTREAMDECODERFACTORY_H
