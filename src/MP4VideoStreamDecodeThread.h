//
// Created by hjzh on 18-2-13.
//

#ifndef GOKU_MP4VIDEOSTREAMDECODETHREAD_H
#define GOKU_MP4VIDEOSTREAMDECODETHREAD_H


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

#include "VideoDecodeThread.h"

///
/// This is a video stream decode thread class
///
///
class MP4VideoStreamDecodeThread : public VideoDecodeThread {
public:
  /// The function to start the thread
  /// \param cb
  void Start(const std::function<FrameCBFunc>& cb);
  /// The function to exit the thread
  ///
  void Exit();
  /// The constructor function
  /// \param config
  /// \param logger
  MP4VideoStreamDecodeThread(const AppConfig& config, Poco::Logger& logger) : VideoDecodeThread(config, logger) {
  }
  virtual ~MP4VideoStreamDecodeThread() {
    if (m_thread.isRunning()) {
      Exit();
    }
  }

  /// The function is used to init a ffmpeg video decoder
  /// \return
  bool Init();
  /// The function is used to connect to a video url
  VID_ERR Connect();

protected:
  virtual void run();

private:
  AVFormatContext *m_pFormatCtx = nullptr; ///< used in decode
  AVCodecContext *m_pDecoderCtx = nullptr;
  enum AVPixelFormat m_hwPixFmt;
  AVCodec *m_pDecoder = nullptr;
  AVPacket m_packet;
  AVStream *m_pVideo = nullptr;
  enum AVHWDeviceType m_hwType = AV_HWDEVICE_TYPE_NONE;
  AVBufferRef *m_phwDevCtx = nullptr;

  int m_nVideoStreamIndex = -1;

  enum AVPixelFormat m_avPixelFormat;
  std::string m_strStreamUrl;

  bool m_bConnected = false;
  Poco::Thread m_thread;
  std::function<FrameCBFunc> m_frameCallback;
  bool m_stop = false;

private:
  enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
                                   const enum AVPixelFormat *pix_fmts);
  int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type);
};

#endif //GOKU_MP4VIDEOSTREAMDECODETHREAD_H
