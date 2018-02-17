project(Goku)

cuda_add_executable(Goku ObjectDetectThread.cc VideoStreamDecodeThread.cc main.cc yuv2bgr.cu
        VideoDetectApp CommonStruct.h ErrCode.h yuv2bgr.h
        ObjectDetectThread.h StringUtility.h NamedMutexScopedLock.h resizeImageGPU.cu resizeImageGPU.h
        VideoStreamDecodeThreadFactory.h VideoStreamDecodeThreadFactory.cc MP4VideoStreamDecodeThread.h
        MP4VideoStreamDecodeThread.cc RTSPStreamDecodeThread.cc RTSPStreamDecodeThread.h USBWebCamStreamDecodeThread.h
        USBWebCamStreamDecodeThread.cc VideoDecodeThread.cc VideoDecodeThread.h)

target_link_libraries(Goku tensorflow_cc
        tensorflow_framework ${PROTOBUF_LIBRARIES}
        avcodec avutil avdevice avfilter avformat swresample swscale postproc
        pthread PocoNet PocoUtil PocoFoundation /usr/local/lib/libopencv_core.so /usr/local/lib/libopencv_cudev.so
        /usr/local/lib/libopencv_photo.so /usr/local/lib/libopencv_shape.so /usr/local/lib/libopencv_video.so
        /usr/local/lib/libopencv_highgui.so /usr/local/lib/libopencv_imgproc.so
        /usr/local/lib/libopencv_videoio.so /usr/local/lib/libopencv_core.so /usr/local/lib/libopencv_cudacodec.so
        /usr/local/lib/libopencv_cudev.so /usr/local/lib/libopencv_imgcodecs.so)