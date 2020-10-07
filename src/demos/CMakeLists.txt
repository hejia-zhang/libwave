#if (WITH_OBJECTDETECTION)
#    add_subdirectory(ObjectDetection)
#endif ()

#if (WITH_VIDEOPLAYER)
#    add_subdirectory(VideoPlayer)
#endif ()

add_subdirectory(VideoStreamingServer)