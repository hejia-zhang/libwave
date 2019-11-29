//
// Created by hejia on 18-2-13.
//

#include "libvsd/VideoStreamDecoders/VideoStreamDecoderFactory.h"

VideoStreamDecoder::Ptr VideoStreamDecoderFactory::MakeStreamDecoder(VID_STREAM_TYPE type) {
  VideoStreamDecoder::Ptr ret;
  switch (type) {
    case USB_WEBCAM: ret = USBWebCamStreamDecoder::Ptr(new USBWebCamStreamDecoder(m_config, m_logger));
      break;
    case MP4_VIDEO: ret = MP4VideoStreamDecoder::Ptr(new MP4VideoStreamDecoder(m_config, m_logger));
      break;
    default: break;
  }

  return ret;
}