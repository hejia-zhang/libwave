//
// Created by hejia on 10/6/20.
//

#include "VideoStreamProcess/VideoPlayingThread.h"


class FrameNotification : public Poco::Notification {
 public:
  typedef Poco::AutoPtr<FrameNotification> Ptr;
  explicit FrameNotification(const ImageFrame &frame) : m_frame(frame) {

  }

  ImageFrame m_frame;
};

void VideoPlayingThread::run() {
  for (;;) {
    if (m_stop) {
      break;
    }

    Poco::Notification::Ptr pNf(m_notiQueue.waitDequeueNotification());
    if (!pNf) {
      break;
    }
    FrameNotification::Ptr pWorkNf = pNf.cast<FrameNotification>();
    try {
      m_logger.debug("Receive one frame");
      cv::imshow("Preview", pWorkNf->m_frame.m_img);
      char c=(char)cv::waitKey(25);
    } catch (const std::exception &e) {
      m_logger.error("VideoPlayingThread::run: %s", e.what());
    }
  }
}

void VideoPlayingThread::Start() {
  if (m_thread.isRunning()) {
    return;
  }

  m_thread.start(*this);
}

void VideoPlayingThread::Exit() {
  m_stop = true;
  m_notiQueue.clear();
  m_notiQueue.wakeUpAll();
  m_thread.join();
}

void VideoPlayingThread::onRecvFrame(const ImageFrame &vf) {
  m_notiQueue.enqueueNotification(new FrameNotification(vf));
}