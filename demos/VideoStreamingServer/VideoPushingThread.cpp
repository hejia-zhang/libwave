#include "VideoPushingThread.h"


void VideoPushingThread::run() {
  for (;;) {
    if (m_stop) {
      break;
    }

    Poco::Notification::Ptr pNf(m_notiQueue.waitDequeueNotification());
    if (!pNf) {
      break;
    }
  }
}

void VideoPushingThread::Start() {
  if (m_thread.isRunning()) {
    return;
  }

  m_thread.start(*this);
}

void VideoPushingThread::Exit() {
  m_stop = true;
  m_notiQueue.clear();
  m_notiQueue.wakeUpAll();
  m_thread.join();
}