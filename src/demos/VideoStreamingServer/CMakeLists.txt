FIND_PACKAGE(Poco REQUIRED COMPONENTS Util Net Foundation)

add_executable(${PROJECT_NAME}_video_streaming_server main.cpp VideoStreamingServerApp.cpp VideoStreamingServerApp.h
        NamedMutexScopedLock.h VideoPushingThread.h VideoPushingThread.cpp)

target_link_libraries(${PROJECT_NAME}_video_streaming_server ${Poco_LIBRARIES} ${PROJECT_NAME})