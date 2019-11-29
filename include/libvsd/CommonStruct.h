//
// Created by hejia on 18-2-6.
//

#ifndef VIDEODETECTDEMO_COMMONSTRUCT_H
#define VIDEODETECTDEMO_COMMONSTRUCT_H
#include <opencv2/opencv.hpp>
#include <Poco/SharedPtr.h>

// The release policy for SharedPtr holding arrays
class ReleaseIplImagePolicy {
public:
  static void release(IplImage *pImg) {
    // Delete the object
    // Note that pObj can be 0;
    cvReleaseImage(&pImg);
  }
};

typedef Poco::SharedPtr<IplImage, Poco::ReferenceCounter, ReleaseIplImagePolicy> SharedIplImage;

struct ImageFrame {
  cv::Mat m_img;
  Poco::Int64 frameIdx;
  ImageFrame() : ImageFrame(cv::Mat()) {}
  ImageFrame(const cv::Mat& img) : m_img(img) {
    frameIdx = 0;
  }
};

struct AppConfig {
  // For video processing
  int m_nTolerableFrameProcessDely;
  std::string m_szHwName;
  std::string m_szVideoStreamAddress;
  bool m_openPrev;
  bool m_ifResize;
  bool m_bgpu_decode;
  int m_resizedWidth;
  int m_resizedHeight;
  int m_videoType;

  // For TF
  std::string m_szLabelPath;
  std::string m_szGraphPath;
  std::string m_szInputLayer;
  std::vector<std::string> m_vecOutputLayers;
};
#endif //VIDEODETECTDEMO_COMMONSTRUCT_H
