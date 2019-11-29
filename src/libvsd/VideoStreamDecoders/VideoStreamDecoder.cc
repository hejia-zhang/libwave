//
// Created by hejia on 18-2-13.
//

#include "libvsd/VideoStreamDecoders/VideoStreamDecoder.h"

VideoStreamDecoder::VideoStreamDecoder(const AppConfig &config, Poco::Logger &logger) :
    m_config(config), m_logger(logger) {
}

VideoStreamDecoder::~VideoStreamDecoder() {
  if (m_thread.isRunning()) {
    Exit();
  }
}

void VideoStreamDecoder::Exit() {
  m_stop = true;
  m_thread.join();
}

void VideoStreamDecoder::Start(const std::function<FrameCBFunc> &cb) {
  m_frameCallback = cb;
  m_thread.start(*this);
}