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

#include <Poco/Thread.h>
#include <Poco/AutoPtr.h>
#include <Poco/Logger.h>
#include <Poco/Runnable.h>

#include "../CommonStruct.h"
#include "../ErrCode.h"
#include "VideoStreamDecoder.h"

using FrameCBFunc = void(const ImageFrame &);

 class USBWebCamStreamDecoder : public VideoStreamDecoder {
 public:
//  typedef Poco::AutoPtr<USBWebCamStreamDecoder> Ptr;

  USBWebCamStreamDecoder(const AppConfig &config) : VideoStreamDecoder(config) {
  }

  virtual ~USBWebCamStreamDecoder() {
  }
//  void Start(const std::function<FrameCBFunc> &cb = std::function<FrameCBFunc>());
//  void Exit();
  virtual void run() override;
  bool Init();
  VID_ERR Connect();

 private:
  AVFormatContext *m_pFormatCtx = nullptr;
  AVCodecContext *m_pDecoderCtx = nullptr;
  enum AVPixelFormat m_hwPixFmt;
  AVCodec *m_pDecoder = nullptr;
  AVStream *m_pVideo = nullptr;
  bool m_stop = false;
  int m_nVideoStreamIndex = -1;
  bool m_bConnected = false;
};
#endif //USBWEBCAMSTREAMDECODER_H
