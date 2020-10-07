# libwave
C++ library for fast prototyping video streaming applications.

## Build Instructions

- Build FFmpeg from source based on this [doc](https://developer.nvidia.com/ffmpeg)
**(Don't forget to run sudo make install after make) 
(Make sure you are using the version of ffmpeg you just build) (recompile with add --enable-shared in ./configure)**
- Do normal cmake build