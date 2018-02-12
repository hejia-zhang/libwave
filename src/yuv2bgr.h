#pragma once

#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_profiler_api.h>
#include <curand.h>

#ifdef __cplusplus
extern "C" {
#endif

  int cvtColor(unsigned char *d_req,
    unsigned char *d_res,
    int resolution,
    int height,
    int width,
    int linesize);

#ifdef __cplusplus
}
#endif