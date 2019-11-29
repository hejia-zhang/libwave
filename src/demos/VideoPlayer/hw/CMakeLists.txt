FIND_PACKAGE(Poco REQUIRED COMPONENTS Util Net Foundation)

add_executable(${PROJECT_NAME}_video_player_hw main.cpp VideoPlayerApp.cpp)

target_link_libraries(${PROJECT_NAME}_video_player_hw ${Poco_LIBRARIES} ${PROJECT_NAME})