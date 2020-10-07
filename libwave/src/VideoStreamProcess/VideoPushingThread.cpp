#include "VideoStreamProcess/VideoPushingThread.h"

static const unsigned char base64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

class FrameNotification : public Poco::Notification {
 public:
  typedef Poco::AutoPtr<FrameNotification> Ptr;
  explicit FrameNotification(const ImageFrame &frame) : m_frame(frame) {

  }

  ImageFrame m_frame;
};


std::string base64_encode(const unsigned char *src, size_t len)
{
  unsigned char *out, *pos;
  const unsigned char *end, *in;

  size_t olen;

  olen = 4*((len + 2) / 3); /* 3-byte blocks to 4-byte */

  if (olen < len)
    return std::string(); /* integer overflow */

  std::string outStr;
  outStr.resize(olen);
  out = (unsigned char*)&outStr[0];

  end = src + len;
  in = src;
  pos = out;
  while (end - in >= 3) {
    *pos++ = base64_table[in[0] >> 2];
    *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
    *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
    *pos++ = base64_table[in[2] & 0x3f];
    in += 3;
  }

  if (end - in) {
    *pos++ = base64_table[in[0] >> 2];
    if (end - in == 1) {
      *pos++ = base64_table[(in[0] & 0x03) << 4];
      *pos++ = '=';
    }
    else {
      *pos++ = base64_table[((in[0] & 0x03) << 4) |
          (in[1] >> 4)];
      *pos++ = base64_table[(in[1] & 0x0f) << 2];
    }
    *pos++ = '=';
  }

  return outStr;
}


void VideoPushingThread::run() {
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
      if (m_connected) {
//        char const *testStr = "Hello websocket client!";
        std::vector<uchar> buffer;
        buffer.resize(static_cast<size_t>(pWorkNf->m_frame.m_img.rows) * static_cast<size_t>(pWorkNf->m_frame.m_img.cols));
        cv::imencode(".jpg", pWorkNf->m_frame.m_img, buffer);
        std::string encoding = base64_encode(buffer.data(), buffer.size());
        m_webSocket->sendFrame(encoding.c_str(), strlen(encoding.c_str()), WebSocket::FRAME_TEXT);
      }
    }catch (const std::exception &e) {
        m_logger.error("VideoPlayingThread::run: %s", e.what());
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

void VideoPushingThread::onRecvFrame(const ImageFrame&vf) {
  m_notiQueue.enqueueNotification(new FrameNotification(vf));
}