//
// Created by hejia on 10/6/20.
//

#ifndef LIBWAVE_VIDEOSTREAMINGSERVERAPP_H
#define LIBWAVE_VIDEOSTREAMINGSERVERAPP_H

#include <Poco/Util/ServerApplication.h>

#include "CommonStruct.h"


class VideoStreamingServerApp : public Poco::Util::ServerApplication {
 public:
    VideoStreamingServerApp() = default;

 protected:
    void initialize(Application &self) {
      loadConfiguration();
      ServerApplication::initialize(self);
    }

    bool ParseConfiguration();

    int main(const std::vector<std::string> &args);

    AppConfig m_config;
};

#endif //LIBWAVE_VIDEOSTREAMINGSERVERAPP_H
