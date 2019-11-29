#cuda_add_executable(DragonFly demos/ObjectDetection/ObjectDetectThread.cc demos/ObjectDetection/main.cc cuda/yuv2bgr.cu
#        VideoDetectApp CommonStruct.h ErrCode.h cuda/yuv2bgr.h
#        demos/ObjectDetection/ObjectDetectThread.h StringUtility.h NamedMutexScopedLock.h cuda/resizeImageGPU.cu cuda/resizeImageGPU.h
#        ../include/VideoStreamDecoders/VideoStreamDecoderFactory.h libvsd/VideoStreamDecoderFactory.cc ../include/VideoStreamDecoders/MP4VideoStreamDecoder.h
#        libvsd/MP4VideoStreamDecoder.cc libvsd/RTSPStreamDecoder.cc ../include/VideoStreamDecoders/RTSPStreamDecoder.h ../include/VideoStreamDecoders/USBWebCamStreamDecoder.h
#        libvsd/USBWebCamStreamDecoder.cc libvsd/VideoStreamDecoder.cc ../include/VideoStreamDecoders/VideoStreamDecoder.h)
#
#      target_link_libraries(DragonFly tensorflow_cc
#        tensorflow_framework ${PROTOBUF_LIBRARIES}
#        avcodec avutil avdevice avfilter avformat swresample swscale postproc
#        pthread PocoNet PocoUtil PocoFoundation /usr/local/lib/libopencv_core.so /usr/local/lib/libopencv_cudev.so
#        /usr/local/lib/libopencv_photo.so /usr/local/lib/libopencv_shape.so /usr/local/lib/libopencv_video.so
#        /usr/local/lib/libopencv_highgui.so /usr/local/lib/libopencv_imgproc.so
#        /usr/local/lib/libopencv_videoio.so /usr/local/lib/libopencv_core.so /usr/local/lib/libopencv_cudacodec.so
#        /usr/local/lib/libopencv_cudev.so /usr/local/lib/libopencv_imgcodecs.so)

add_subdirectory(libvsd)

add_subdirectory(demos)
