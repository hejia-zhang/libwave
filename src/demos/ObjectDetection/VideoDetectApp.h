//
// Created by hejia on 18-2-6.
//

#ifndef VIDEODETECTDEMO_VIDEODETECTAPP_H
#define VIDEODETECTDEMO_VIDEODETECTAPP_H

#include "Poco/Util/ServerApplication.h"
#include "../../../inc/libvsd/CommonStruct.h"

///
/// This is the main application class
///
class VideoDetectApp : public Poco::Util::ServerApplication {
 public:
  VideoDetectApp();

 protected:
  // Initialize the app
  void initialize(Application &self) {
    loadConfiguration(); // load default configuration files, if present
    ServerApplication::initialize(self);
  }

  bool ParseConfiguration();

  int main(const std::vector<std::string> &args);

  AppConfig m_config;

 private:
  bool ParseTFConfiguration();
  bool ParseVideoConfiguration();

  bool GetOutputLayers(const std::string &szOutLayers, std::vector<std::string> &outputlayers);
};
#endif //VIDEODETECTDEMO_VIDEODETECTAPP_H
