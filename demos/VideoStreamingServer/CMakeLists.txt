FIND_PACKAGE(Poco PATHS /usr/loca/lib/cmake REQUIRED COMPONENTS Foundation Util Net)

include_directories(../../libwave/include)

add_executable(${PROJECT_NAME}_video_streaming_server main.cpp VideoStreamingServerApp.cpp VideoStreamingServerApp.h
        NamedMutexScopedLock.h VideoPushingThread.h VideoPushingThread.cpp)
target_link_libraries(${PROJECT_NAME}_video_streaming_server ${Poco_LIBRARIES} ${PROJECT_NAME})