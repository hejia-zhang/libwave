//
// Created by hejia on 10/6/20.
//

#ifndef WAVE_VIDEOPLAYINGTHREAD_H
#define WAVE_VIDEOPLAYINGTHREAD_H

#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include <Poco/Logger.h>
#include <Poco/NotificationQueue.h>

#include "CommonStruct.h"
#include "ErrCode.h"
#include "StringUtility.h"

class VideoPlayingThread : public Poco::Runnable, public Poco::RefCountedObject {
 public:
  VideoPlayingThread(const AppConfig &config, Poco::Logger &logger) : m_config(config), m_logger(logger) {

  }

  virtual ~VideoPlayingThread() {
    if (m_thread.isRunning()) {
      Exit();
    }
  }

  void Start();
  void Exit();

  void onRecvFrame(const ImageFrame&);

 protected:
  virtual void run() override;

 protected:
  const AppConfig &m_config;
  Poco::Logger &m_logger;

 private:
  /// These member variables are related to thread
  Poco::Thread m_thread;
  bool m_stop = false;
  Poco::NotificationQueue m_notiQueue;
};

#endif //WAVE_VIDEOPLAYINGTHREAD_H
