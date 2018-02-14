//
// Created by hjzh on 18-2-13.
//

#ifndef GOKU_VIDEODETECTTHREAD_H
#define GOKU_VIDEODETECTTHREAD_H

#include <Poco/Logger.h>
#include "CommonStruct.h"

class VideoDetectThread {
public:
  VideoDetectThread(const AppConfig& config, Poco::Logger& logger);
  virtual ~VideoDetectThread();

protected:
  const AppConfig& m_config;
  Poco::Logger& m_logger;
};

#endif //GOKU_VIDEODETECTTHREAD_H
