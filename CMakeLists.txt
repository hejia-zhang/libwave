cmake_minimum_required(VERSION 3.5.1)

project(DragonFly)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CUDA_TOOLKIT_ROOT_DIR /usr/local/cuda)

# find packages
find_package(Eigen3 REQUIRED)
find_package(CUDA QUIET REQUIRED)

set(PROTOBUF_INCLUDE_DIRS /usr/local/include/google)

set(PROTOBUF_LIBRARIES /usr/local/lib/libprotobuf.so)

include_directories(/usr/local/include/google/tensorflow
                    ${PROTOBUF_INCLUDE_DIRS}
                    ${EIGEN3_INCLUDE_DIRS}
                    /home/hjzh/Library_Sources/3rd_library/tensorflow/bazel-tensorflow/external/nsync/public
                    /usr/local/include
                    /usr/local/cuda/include)

link_directories(/usr/local/lib)

add_subdirectory(src)



