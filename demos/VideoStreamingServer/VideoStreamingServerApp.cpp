#include "VideoStreamDecoders/VideoStreamDecoderFactory.h"
#include "VideoStreamingServerApp.h"
#include "VideoStreamDecoders/USBWebCamStreamDecoder.h"
#include "NamedMutexScopedLock.h"
#include "VideoStreamProcess/VideoPlayingThread.h"
#include "VideoProcessEngine.h"
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


class PageRequestHandler: public HTTPRequestHandler
  /// Return a HTML document with some JavaScript creating
  /// a WebSocket connection.
{
 public:
  void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
  {
    response.setChunkedTransferEncoding(true);
    response.setContentType("text/html");
    std::ostream& ostr = response.send();
    ostr << "<html>";
    ostr << "<head>";
    ostr << "<title>WebSocketServer</title>";
    ostr << "<script type=\"text/javascript\">";
    ostr << "function WebSocketTest()";
    ostr << "{";
    ostr << "  if (\"WebSocket\" in window)";
    ostr << "  {";
    ostr << "    var ws = new WebSocket(\"ws://" << request.serverAddress().toString() << "/ws\");";
    ostr << "    ws.onopen = function()";
    ostr << "      {";
    ostr << "        ws.send(\"Hello, world!\");";
    ostr << "      };";
    ostr << "    ws.onmessage = function(evt)";
    ostr << "      { ";
    ostr << "        var msg = evt.data;";
    ostr << "        alert(\"Message received: \" + msg);";
    ostr << "        ws.close();";
    ostr << "      };";
    ostr << "    ws.onclose = function()";
    ostr << "      { ";
    ostr << "        alert(\"WebSocket closed.\");";
    ostr << "      };";
    ostr << "  }";
    ostr << "  else";
    ostr << "  {";
    ostr << "     alert(\"This browser does not support WebSockets.\");";
    ostr << "  }";
    ostr << "}";
    ostr << "</script>";
    ostr << "</head>";
    ostr << "<body>";
    ostr << "  <h1>WebSocket Server</h1>";
    ostr << "  <p><a href=\"javascript:WebSocketTest()\">Run WebSocket Script</a></p>";
    ostr << "</body>";
    ostr << "</html>";
  }
};


class WebSocketRequestHandler: public HTTPRequestHandler
  /// Handle a WebSocket connection.
{
 public:
  WebSocketRequestHandler(const std::function<void(WebSocket* ws)>& constructed_cb,
                          const std::function<void(void)>& disconstructed_cb) : HTTPRequestHandler(),
                                                                                m_constructed_cb(constructed_cb),
                                                                                m_disconstructed_cb(disconstructed_cb) {

  }
  void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
  {
    Application& app = Application::instance();
    try
    {
      WebSocket ws(request, response);
      app.logger().information("WebSocket connection established.");
      m_constructed_cb(&ws);
      char buffer[1024];
      int flags;
      int n;
      do
      {
        n = ws.receiveFrame(buffer, sizeof(buffer), flags);
        app.logger().information(Poco::format("Frame received (length=%d, flags=0x%x).", n, unsigned(flags)));
        ws.sendFrame(buffer, n, flags);
      }
      while (n > 0 && (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);
      m_disconstructed_cb();
      app.logger().information("WebSocket connection closed.");
    }
    catch (WebSocketException& exc)
    {
      m_disconstructed_cb();
      app.logger().log(exc);
      switch (exc.code())
      {
        case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
          response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
          // fallthrough
        case WebSocket::WS_ERR_NO_HANDSHAKE:
        case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
        case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
          response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
          response.setContentLength(0);
          response.send();
          break;
      }
    }
  }

  std::function<void(WebSocket* ws)> m_constructed_cb;
  std::function<void(void)> m_disconstructed_cb;
};

class RequestHandlerFactory: public HTTPRequestHandlerFactory
{
 public:
  RequestHandlerFactory(const std::function<void(WebSocket* ws)>& constructed_cb,
      const std::function<void(void)>& disconstructed_cb) : HTTPRequestHandlerFactory(), m_constructed_cb(constructed_cb),
      m_disconstructed_cb(disconstructed_cb) {

  }
  HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request)
  {
    Application& app = Application::instance();
    app.logger().information("Request from "
                                 + request.clientAddress().toString()
                                 + ": "
                                 + request.getMethod()
                                 + " "
                                 + request.getURI()
                                 + " "
                                 + request.getVersion());

    for (HTTPServerRequest::ConstIterator it = request.begin(); it != request.end(); ++it)
    {
      app.logger().information(it->first + ": " + it->second);
    }

    if(request.find("Upgrade") != request.end() && Poco::icompare(request["Upgrade"], "websocket") == 0)
      return new WebSocketRequestHandler(m_constructed_cb, m_disconstructed_cb);
    else
      return new PageRequestHandler;
  }

  std::function<void(WebSocket* ws)> m_constructed_cb;
  std::function<void(void)> m_disconstructed_cb;
};


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

//  VideoStreamDecoderFactory videoStreamDecoderFactory(m_config);
//  VideoStreamDecoder::Ptr
//      pVideoStreamDecoder = videoStreamDecoderFactory.MakeStreamDecoder((VID_STREAM_TYPE) m_config.m_streamType);
  VideoProcessEngine videoProcessEngine(m_config, logger());

  USBWebCamStreamDecoder pVideoStreamDecoder(m_config);
  if (!pVideoStreamDecoder.Init()) {
    logger().error("Failed to initialize VideoStreamDecoder!");
    return EXIT_SOFTWARE;
  }
  std::function<void(const ImageFrame&)> onFrameDecodedCbFunc = std::bind(&VideoProcessEngine::onRecvFrame,
      &videoProcessEngine,
      std::placeholders::_1);
  pVideoStreamDecoder.Start(onFrameDecodedCbFunc);

  // we need a callback function when we construct a websocket connection.
  std::function<void(WebSocket* ws)> onSocketConstructed = std::bind(&VideoProcessEngine::onSocketConstructed,
      &videoProcessEngine,
      std::placeholders::_1);

  std::function<void(void)> onSocketDestroyed = std::bind(&VideoProcessEngine::onSocketDisconnected,
                                                                     &videoProcessEngine);

  unsigned short port = (unsigned short) config().getInt("WebSocketServer.port", 20020);
  ServerSocket svs(port);
  auto *requestHandlerFactoryPtr = new RequestHandlerFactory(onSocketConstructed, onSocketDestroyed);
  HTTPServer srv(requestHandlerFactoryPtr, svs, new HTTPServerParams);
  srv.start();

  waitForTerminationRequest();

  srv.stop();
}