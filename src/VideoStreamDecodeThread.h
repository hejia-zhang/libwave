//
// Created by hjzh on 18-2-5.
//
#ifndef VIDEODETECTDEMO_VIDEOSTREAMDECODERTHREAD_H
#define VIDEODETECTDEMO_VIDEOSTREAMDECODERTHREAD_H

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

#include <string>
#include "ErrCode.h"

// For Poco
#include "Poco/AutoPtr.h"
#include "Poco/Delegate.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "Poco/RefCountedObject.h"
#include "Poco/Logger.h"
#include "Poco/Timer.h"

#include "CommonStruct.h"

using FrameCBFunc = void(const ImageFrame&);

class VideoStreamDecodeThread : public Poco::Runnable, public Poco::RefCountedObject {
public:
  typedef Poco::AutoPtr<VideoStreamDecodeThread> Ptr;
  void Start(const std::function<FrameCBFunc>& cb);
  void Exit();
  VideoStreamDecodeThread(const AppConfig& config, Poco::Logger& logger) : m_logger(logger), m_config(config) {
  }
  virtual ~VideoStreamDecodeThread() {
    if (m_thread.isRunning()) {
      Exit();
    }
  }

  bool Init();
  ErrCode Connect();

protected:
  virtual void run();
  const AppConfig& m_config;
  Poco::Logger& m_logger;

private:
  AVFormatContext *m_pFormatCtx = nullptr;
  AVCodecContext *m_pDecoderCtx = nullptr;
  AVCodec *m_pDecoder = nullptr;
  AVPacket m_packet;
  AVStream *m_pVideo = nullptr;
  enum AVHWDeviceType m_hwType = AV_HWDEVICE_TYPE_NONE;

  int m_nVideoStreamIndex = -1;

  enum AVPixelFormat m_avPixelFormat;
  std::string m_strStreamUrl;
  std::string m_strHwName;

  bool m_bConnected = false;
  Poco::Thread m_thread;
  std::function<FrameCBFunc> m_frameCallback;
  bool m_stop = false;

private:
  enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
                                   const enum AVPixelFormat *pix_fmts);
};
#endif //VIDEODETECTDEMO_VIDEOSTREAMDECODER_H
