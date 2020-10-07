set(FFMPEG_DIR cmake)
FIND_PACKAGE(FFMPEG REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED)

INCLUDE_DIRECTORIES(${FFMPEG_INCLUDE_DIR})

cuda_add_library(${PROJECT_NAME} SHARED
        VideoStreamDecoders/MP4VideoStreamDecoder.cc
        VideoStreamDecoders/RTSPStreamDecoder.cc
        VideoStreamDecoders/USBWebCamStreamDecoder.cc
        VideoStreamDecoders/VideoStreamDecoder.cc
        VideoStreamDecoders/VideoStreamDecoderFactory.cc
        cuda/resizeImageGPU.cu
        cuda/yuv2bgr.cu)

target_link_libraries(${PROJECT_NAME} ${FFMPEG_LIBRARIES} ${OpenCV_LIBS})