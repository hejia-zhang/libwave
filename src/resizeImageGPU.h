#pragma once

#include "cuda_runtime.h"  

#ifdef __cplusplus
extern "C" {
#endif

  void resizeImageGPU(unsigned char *d_res,
                      const unsigned char *d_req,
                      int n_resStep,
                      int n_reqStep,
                      int n_resHeight,
                      int n_resWidth,
                      int n_reqHeight,
                      int n_reqWidth);
#ifdef __cplusplus
}
#endif