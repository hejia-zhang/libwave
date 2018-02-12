project(Goku)

cuda_add_executable(Goku ObjectDetectThread.cc VideoStreamDecodeThread.cc main.cc yuv2bgr.cu
        VideoDetectApp CommonStruct.h ErrCode.h yuv2bgr.h
        ObjectDetectThread.h StringUtility.h NamedMutexScopedLock.h)

target_link_libraries(Goku /usr/local/lib/libtensorflow_cc.so
        /usr/local/lib/libtensorflow_framework.so ${PROTOBUF_LIBRARIES}
        avcodec avutil avdevice avfilter avformat swresample swscale postproc
        pthread PocoNet PocoUtil PocoFoundation opencv_core opencv_cudev
        opencv_photo opencv_shape opencv_video opencv_highgui opencv_imgproc
        opencv_videoio opencv_core opencv_cudacodec opencv_cudev)