#ifndef LIBWAVE_VIDEOPUSHINGTHREAD_H
#define LIBWAVE_VIDEOPUSHINGTHREAD_H

#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include <Poco/Logger.h>
#include "Poco/NotificationQueue.h"

#include "../../libwave/include/CommonStruct.h"
#include "../../libwave/include/ErrCode.h"
#include "../../libwave/include/StringUtility.h"

class VideoPushingThread : public Poco::Runnable, public Poco::RefCountedObject {
 public:
  typedef Poco::AutoPtr<VideoPushingThread> Ptr;

  VideoPushingThread(const AppConfig &config, Poco::Logger &logger) : m_config(config), m_logger(logger) {

  }

  virtual ~VideoPushingThread() {
    if (m_thread.isRunning()) {
      Exit();
    }
  }

  void Start();
  void Exit();

 protected:
  void run() override;

 public:

 protected:
  const AppConfig &m_config;
  Poco::Logger &m_logger;

 private:
  /// These member variables are related to thread
  Poco::Thread m_thread;
  bool m_stop = false;
  Poco::NotificationQueue m_notiQueue;
};

#endif //LIBWAVE_VIDEOPUSHINGTHREAD_H
