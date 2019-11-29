FILE(GLOB ${PROJECT_NAME}_object_detection_source *.cc)

add_executable(${PROJECT_NAME}_object_detection ${${PROJECT_NAME}_object_detection_source})

target_link_libraries(${PROJECT_NAME}_object_detection vsd)