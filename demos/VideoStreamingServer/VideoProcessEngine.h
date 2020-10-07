//
// Created by hejia on 10/6/20.
//

#ifndef WAVE_VIDEOPROCESSENGINE_H
#define WAVE_VIDEOPROCESSENGINE_H

#include <Poco/Logger.h>

#include "CommonStruct.h"
#include "VideoStreamProcess/VideoPlayingThread.h"


class VideoProcessEngine {
 public:
  VideoProcessEngine(const AppConfig &config, Poco::Logger &logger) : m_config(config), m_logger(logger),
    m_videoPlayingThread(config, logger) {
    m_videoPlayingThread.Start();
  }

  void onRecvFrame(const ImageFrame& imageFrame) {
    m_videoPlayingThread.onRecvFrame(imageFrame);
  }

 protected:
  const AppConfig &m_config;
  Poco::Logger &m_logger;

 private:
  VideoPlayingThread m_videoPlayingThread;
};

#endif //WAVE_VIDEOPROCESSENGINE_H
