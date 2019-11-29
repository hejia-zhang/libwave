//
// Created by hejia on 18-2-13.
//

#ifndef USBWEBCAMSTREAMDECODER_H
#define USBWEBCAMSTREAMDECODER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#include <libavutil/opt.h>
#include <libavutil/avassert.h>
#include <libavutil/imgutils.h>
#include <libavfilter/avfilter.h>
#include <libavdevice/avdevice.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include "VideoStreamDecoder.h"

class USBWebCamStreamDecoder : public VideoStreamDecoder {
public:
  USBWebCamStreamDecoder(const AppConfig& config, Poco::Logger& logger) : VideoStreamDecoder(config, logger) {
  }

  virtual ~USBWebCamStreamDecoder() {
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
#endif //USBWEBCAMSTREAMDECODER_H
