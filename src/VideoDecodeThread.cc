//
// Created by hjzh on 18-2-13.
//

#include "VideoDecodeThread.h"

VideoDecodeThread::VideoDecodeThread(const AppConfig& config, Poco::Logger& logger) :
    m_config(config), m_logger(logger){
}

VideoDecodeThread::~VideoDecodeThread(){
  if (m_thread.isRunning()) {
    Exit();
  }
}

void VideoDecodeThread::Exit() {
  m_stop = true;
  m_thread.join();
}

void VideoDecodeThread::Start(const FrameCBFunc &cb) {
  m_frameCallback = cb;
  m_thread.start(*this);
}