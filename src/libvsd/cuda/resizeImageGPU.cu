#include "../VideoStreamDecoders/resizeImageGPU.h"
#include "device_launch_parameters.h"

__global__ void kernel(unsigned char* d_res, const unsigned char* d_req, int n_resStep,
                       int n_reqStep, int n_resHeight, int n_resWidth, int n_reqHeight,
                       int n_reqWidth)
{
  int threadid = blockDim.x * blockIdx.x + threadIdx.x;

  int i = threadid / n_resWidth;
  int j = threadid % n_resWidth;
  
  double fRows = n_resHeight / (float)n_reqHeight;
  double fCols = n_resWidth / (float)n_reqWidth;

  int pX = 0;
  int pY = 0;

  pX = (int)(i / fRows);
  pY = (int)(j / fCols);
  if (pX < n_reqHeight && pX >= 0 && pY < n_reqWidth && pY >= 0)
  {
    *(d_res + i * n_resStep + 3 * j + 0) = *(d_req + pX * n_reqStep + 3 * pY + 0);
    *(d_res + i * n_resStep + 3 * j + 1) = *(d_req + pX * n_reqStep + 3 * pY + 1);
    *(d_res + i * n_resStep + 3 * j + 2) = *(d_req + pX * n_reqStep + 3 * pY + 2);
  }
}

void resizeImageGPU(unsigned char* d_res, const unsigned char* d_req, int n_resStep, 
                    int n_reqStep, int n_resHeight, int n_resWidth, int n_reqHeight, 
                    int n_reqWidth)
{
  int threadsPerBlock = 256;
  int blocksPerGrid = (n_resWidth * n_resHeight + threadsPerBlock - 1) / threadsPerBlock;
  kernel << < blocksPerGrid, threadsPerBlock>> > (d_res, d_req, n_resStep, n_reqStep,
                            n_resHeight, n_resWidth, n_reqHeight,
                            n_reqWidth);
}
