//
// Created by hejia on 18-2-13.
//

#include "VideoStreamDecoders/VideoStreamDecoderFactory.h"

VideoStreamDecoder::Ptr VideoStreamDecoderFactory::MakeStreamDecoder(VID_STREAM_TYPE type) {
  VideoStreamDecoder::Ptr ret;
  switch (type) {
    case USB_WEBCAM: ret = USBWebCamStreamDecoder::Ptr(new USBWebCamStreamDecoder(m_config));
      break;
    case MP4_VIDEO: ret = MP4VideoStreamDecoder::Ptr(new MP4VideoStreamDecoder(m_config));
      break;
    default: break;
  }

  return ret;
}