cmake_minimum_required(VERSION 3.5.1)

project(wave)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CUDA_TOOLKIT_ROOT_DIR /usr/local/cuda)

# find packages
find_package(Eigen3 REQUIRED)
find_package(CUDA QUIET REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

include_directories(include)

include_directories(${EIGEN3_INCLUDE_DIRS}
                    /usr/local/include
                    /usr/local/cuda/include)

include_directories(3rd_party/websocketpp 3rd_party/beast/include/boost)

link_directories(/usr/local/lib)

add_subdirectory(libwave)

add_subdirectory(demos)



