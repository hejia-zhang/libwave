#ifndef LIBWAVE_VIDEOPUSHINGTHREAD_H
#define LIBWAVE_VIDEOPUSHINGTHREAD_H

#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include <Poco/Logger.h>
#include "Poco/NotificationQueue.h"
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

#include "CommonStruct.h"
#include "ErrCode.h"
#include "StringUtility.h"


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


class VideoPushingThread : public Poco::Runnable, public Poco::RefCountedObject {
 public:
  typedef Poco::AutoPtr<VideoPushingThread> Ptr;

  VideoPushingThread(const AppConfig &config, Poco::Logger &logger) : m_config(config), m_logger(logger) {
  }

  virtual ~VideoPushingThread() {
    if (m_thread.isRunning()) {
      Exit();
    }
  }

  void Start();
  void Exit();

 protected:
  virtual void run() override;

 public:
  void onSocketConstructed(WebSocket* wsPtr) {
    m_webSocket = wsPtr;
    m_connected = true;
  }

  void onSocketDisconnected() {
    m_webSocket = nullptr;
    m_connected = false;
  }

  void onRecvFrame(const ImageFrame&);

 protected:
  const AppConfig &m_config;
  Poco::Logger &m_logger;

 private:
  /// These member variables are related to thread
  Poco::Thread m_thread;
  bool m_stop = false;

  Poco::NotificationQueue m_notiQueue;

  WebSocket *m_webSocket = nullptr;
  bool m_connected = false;
};

#endif //LIBWAVE_VIDEOPUSHINGTHREAD_H
