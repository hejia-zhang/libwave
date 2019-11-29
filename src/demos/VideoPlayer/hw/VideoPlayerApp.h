//
// Created by hejia on 11/29/19.
//

#ifndef LIBVSD_VIDEOPLAYERAPP_H
#define LIBVSD_VIDEOPLAYERAPP_H

#include <Poco/Util/ServerApplication.h>

#include "libvsd/CommonStruct.h"

/*
 * This is the main application class
 */
class VideoPlayerApp : public Poco::Util::ServerApplication {
 public:
  VideoPlayerApp() = default;

 protected:
  void intilaize(Application &self) {
    loadConfiguration();
    ServerApplication::initialize(self);
  }

  bool ParseConfiguration();

  int main(const std::vector<std::string> &args);

  AppConfig m_config;
};

#endif //LIBVSD_VIDEOPLAYERAPP_H
