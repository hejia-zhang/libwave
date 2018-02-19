//
// Created by hjzh on 18-2-13.
//

#ifndef GOKU_USBWEBCAMSTREAMDECODETHREAD_H
#define GOKU_USBWEBCAMSTREAMDECODETHREAD_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/pixdesc.h"
#include "libavutil/hwcontext.h"
#include "libavutil/opt.h"
#include "libavutil/avassert.h"
#include "libavutil/imgutils.h"
#include "libavfilter/avfilter.h"
#include "libavdevice/avdevice.h"
}

#include "VideoDecodeThread.h"

class USBWebCamStreamDecodeThread : public VideoDecodeThread {
public:
  USBWebCamStreamDecodeThread(const AppConfig& config, Poco::Logger& logger) : VideoDecodeThread(config, logger) {
  }

  virtual ~USBWebCamStreamDecodeThread() {
  }

  void run();
  bool Init();
  VID_ERR Connect();

private:
  AVFormatContext *m_pFormatCtx = nullptr;
  AVCodecContext *m_pDecoderCtx = nullptr;
  enum AVPixelFormat m_hwPixFmt;
  AVCodec *m_pDecoder = nullptr;
  AVStream *m_pVideo = nullptr;

  int m_nVideoStreamIndex = -1;
};
#endif //GOKU_USBWEBCAMSTREAMDECODETHREAD_H
