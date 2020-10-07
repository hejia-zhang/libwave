//
// Created by hejia on 10/6/20.
//

#ifndef WAVE_VIDEOPROCESSENGINE_H
#define WAVE_VIDEOPROCESSENGINE_H

#include <Poco/Logger.h>

#include "CommonStruct.h"
#include "VideoStreamProcess/VideoPlayingThread.h"
#include "VideoStreamProcess/VideoPushingThread.h"

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Format.h"
#include <iostream>


using Poco::Net::ServerSocket;
using Poco::Net::WebSocket;
using Poco::Net::WebSocketException;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Timestamp;
using Poco::ThreadPool;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;

class VideoProcessEngine {
 public:
  VideoProcessEngine(const AppConfig &config, Poco::Logger &logger) : m_config(config), m_logger(logger),
    m_videoPlayingThread(config, logger), m_videoPushingThread(config, logger) {
    m_videoPlayingThread.Start();
    m_videoPushingThread.Start();
  }

  void onRecvFrame(const ImageFrame& imageFrame) {
    m_videoPlayingThread.onRecvFrame(imageFrame);
    m_videoPushingThread.onRecvFrame(imageFrame);
  }

  void onSocketConstructed(WebSocket* wsPtr) {
    m_videoPushingThread.onSocketConstructed(wsPtr);
  }

  void onSocketDisconnected() {
    m_videoPushingThread.onSocketDisconnected();
  }

 protected:
  const AppConfig &m_config;
  Poco::Logger &m_logger;

 private:
  VideoPlayingThread m_videoPlayingThread;
  VideoPushingThread m_videoPushingThread;

};

#endif //WAVE_VIDEOPROCESSENGINE_H
