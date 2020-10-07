set(FFMPEG_DIR cmake)
FIND_PACKAGE(FFMPEG REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED)

INCLUDE_DIRECTORIES(${FFMPEG_INCLUDE_DIR} include)

cuda_add_library(${PROJECT_NAME} SHARED
        src/VideoStreamDecoders/MP4VideoStreamDecoder.cc
        src/VideoStreamDecoders/RTSPStreamDecoder.cc
        src/VideoStreamDecoders/USBWebCamStreamDecoder.cc
        src/VideoStreamDecoders/VideoStreamDecoder.cc
        src/VideoStreamDecoders/VideoStreamDecoderFactory.cc
        src/cuda/resizeImageGPU.cu
        src/cuda/yuv2bgr.cu
        include/CommonStruct.h
        include/ErrCode.h
        include/StringUtility.h
        include/VideoStreamDecoders/MP4VideoStreamDecoder.h
        include/VideoStreamDecoders/RTSPStreamDecoder.h
        include/VideoStreamDecoders/USBWebCamStreamDecoder.h
        include/VideoStreamDecoders/VideoStreamDecoder.h
        include/VideoStreamDecoders/VideoStreamDecoderFactory.h
        include/VideoStreamProcess/VideoPushingThread.h
        src/VideoStreamProcess/VideoPushingThread.cpp
        src/VideoStreamProcess/VideoPlayingThread.cpp
        include/VideoStreamProcess/VideoPlayingThread.h)

target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBRARIES} ${OpenCV_LIBS})