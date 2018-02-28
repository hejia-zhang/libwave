//
// Created by hjzh on 18-2-13.
//

#include "VideoStreamDecodeThreadFactory.h"

VideoDecodeThread::Ptr VideoStreamDecodeThreadFactory::MakeDecodeThread(VID_STREAM_TYPE type) {
  VideoDecodeThread::Ptr ret;
  switch (type) {
    case USB_WEBCAM: ret = USBWebCamStreamDecodeThread::Ptr(new USBWebCamStreamDecodeThread(m_config, m_logger));
                      break;
    case MP4_VIDEO: ret = MP4VideoStreamDecodeThread::Ptr(new MP4VideoStreamDecodeThread(m_config, m_logger));
                    break;
    default: break;
  }

  return ret;
}