//
// Created by hjzh on 18-2-13.
//

#include "VideoDetectThread.h"

VideoDetectThread::VideoDetectThread(const AppConfig& config, Poco::Logger& logger) :
    m_config(config), m_logger(logger){
}

VideoDetectThread::~VideoDetectThread(){

}