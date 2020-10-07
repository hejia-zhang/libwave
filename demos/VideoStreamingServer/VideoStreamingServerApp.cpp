#include "VideoStreamDecoders/VideoStreamDecoderFactory.h"
#include "VideoStreamingServerApp.h"
#include "NamedMutexScopedLock.h"

bool VideoStreamingServerApp::ParseConfiguration() {
  bool res = true;

  try {
    m_config.m_nTolerableFrameProcessDely = config().getInt("tolerable_frame_delay");
    m_config.m_szVideoStreamAddress = config().getString("video_stream_address");
    m_config.m_szHwName = config().getString("hw_name");
    m_config.m_openPrev = config().getBool("preview_open");
    m_config.m_bgpu_decode = config().getBool("gpu_decode");
    m_config.m_streamType = config().getInt("video_stream_type");
    /// Check if we need to resize the input video image
    m_config.m_ifResize = config().getBool("flag_resized");
    if (m_config.m_ifResize) {
      m_config.m_resizedWidth = config().getInt("resized_width");
      m_config.m_resizedHeight = config().getInt("resized_height");
    }
  } catch (const Poco::NotFoundException &e) {
    logger().error("VideoStreamingServerApp::ParseConfiguration: Can't find some required parameters! "
                   "Please set your Video Process parameters in properties file completely!");
    res = false;
  } catch (const std::exception &e) {
    logger().error("VideoStreamingServerApp::ParseConfiguration: Some errors happended");
    res = false;
  }

  return res;
}

int VideoStreamingServerApp::main(const std::vector<std::string> &args) {
  std::string strInstanceName = "libvsd_video_streaming_server";
  Poco::NamedMutex mutex(strInstanceName);
  NamedMutexScopedLock lock(mutex);

  if (!lock.tryLock()) {
    logger().error(
        "There is one running libvsd_video_streaming_server instance! Only one instance is permitted to run! Kill it first!");
    return EXIT_OK;
  }

  if (!ParseConfiguration()) {
    logger().error("Failed to parse properties file!");
    return EXIT_CONFIG;
  }

  VideoStreamDecoderFactory videoStreamDecoderFactory(m_config);
  VideoStreamDecoder::Ptr
      pVideoStreamDecoder = videoStreamDecoderFactory.MakeStreamDecoder((VID_STREAM_TYPE) m_config.m_streamType);
  if (!pVideoStreamDecoder->Init()) {
    logger().error("Failed to initialize VideoStreamDecoder!");
    return EXIT_SOFTWARE;
  }
  pVideoStreamDecoder->Start();
}