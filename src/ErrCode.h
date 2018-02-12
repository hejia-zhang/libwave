//
// Created by hjzh on 18-2-6.
//

#ifndef VIDEODETECTDEMO_ERRCODE_H
#define VIDEODETECTDEMO_ERRCODE_H
/// The whole application error code
/// Like success, some common errors
enum ErrCode {
  RES_SUC = 0,
  RES_NO_HW = 1,    // can't find hw
  RES_ERR_OPEN_INPUT = 2, // can't open input
  RES_ERR_FIND_STREAM_INFO = 3, // can't find stream info
  RES_ERR_FIND_VIDEO_STREAM = 4,  // can't find a stream
  RES_ERR_NOT_SUPPORT_DEVICE_TYPE = 5, // decoder does not support device type
  RES_NOT_FIND_DECODER = 6, // can't find decoder
};

/// The error code for video processing.
/// From video to frame image.
enum VID_ERR {

};

/// The error code in TF object detect.
/// From frame image to result.
enum TF_ERR {
  RES_TF_OK = 0,
  RES_TF_LOAD_GRAPH = 1,
};

#endif //VIDEODETECTDEMO_ERRCODE_H
