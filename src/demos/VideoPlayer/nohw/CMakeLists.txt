FIND_PACKAGE(OpenCV)

add_executable(${PROJECT_NAME}_video_player_nohw main.cpp)

include_directories(${OpenCV_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}_video_player_nohw ${OpenCV_LIBS})