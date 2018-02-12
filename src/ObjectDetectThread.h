//
// Created by hjzh on 18-2-8.
//
/// This file is about a object detect thread
/// We use tensorflow to detect object

#ifndef VIDEODETECTDEMO_OBJECTDETECTOR_H
#define VIDEODETECTDEMO_OBJECTDETECTOR_H

#include "Poco/Thread.h"
#include "Poco/Runnable.h"
#include "Poco/Logger.h"
#include "Poco/AutoPtr.h"
#include "Poco/NotificationQueue.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/cc/ops/const_op.h"
#include "tensorflow/cc/ops/image_ops.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/graph/default_device.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/lib/core/threadpool.h"
#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/platform/init_main.h"
#include "tensorflow/core/util/command_line_flags.h"
#include "CommonStruct.h"
#include "ErrCode.h"

using ResultCBFunc = void();

class ObjectDetectThread : public Poco::Runnable, public Poco::RefCountedObject{
public:
  typedef Poco::AutoPtr<ObjectDetectThread> Ptr;

  ObjectDetectThread(const AppConfig& config, Poco::Logger& logger) : m_config(config), m_logger(logger){
  }
  virtual ~ObjectDetectThread() {
    if (m_thread.isRunning()) {
      Exit();
    }
  }

  void AddFrame(const ImageFrame& vf);
  void Start(const std::function<ResultCBFunc>& cb);
  void Start();
  void Exit();

  /// The function is used to load graph, initialize the session
  TF_ERR Init();

protected:
  virtual void run() override;

public:
  int Detect(const ImageFrame& imgFrame);

protected:
  const AppConfig& m_config;
  Poco::Logger& m_logger;

private:
  std::unique_ptr<tensorflow::Session> m_session;

private:
  /// These member variables are related to thread
  Poco::Thread m_thread;
  std::function<ResultCBFunc> m_cb;
  bool m_stop = false;
  Poco::NotificationQueue m_notiQueue;
};

#endif //VIDEODETECTDEMO_OBJECTDETECTOR_H
