//
// Created by hjzh on 18-2-13.
//

#ifndef GOKU_VIDEOSTREAMDECODETHREADFACTORY_H
#define GOKU_VIDEOSTREAMDECODETHREADFACTORY_H

enum VID_STREAM_TYPE {
  MP4_VIDEO = 1,
  RTSP_STREAM = 2,
  USB_WEBCAM = 3,
};
class VideoStreamDecodeThreadFactory {
public:
  VideoStreamDecodeThreadFactory();
  virtual ~VideoStreamDecodeThreadFactory();

  void MakeThread(VID_STREAM_TYPE type);
};

#endif //GOKU_VIDEOSTREAMDECODETHREADFACTORY_H
