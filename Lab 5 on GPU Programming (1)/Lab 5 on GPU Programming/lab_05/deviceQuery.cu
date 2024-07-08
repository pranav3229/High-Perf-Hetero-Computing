/*
 ============================================================================
 Author        : G. Barlas
 Version       : 1.0
 Last modified : December 2014
 License       : Released under the GNU GPL 3.0
 Description   : 
 To build use  : nvcc deviceQuery.cu -o deviceQuery
 ============================================================================
 */
#include <stdio.h>
#include <cuda.h>

int main ()
{
  int deviceCount = 0;
  cudaGetDeviceCount (&deviceCount);
  if (deviceCount == 0)
    printf ("No CUDA compatible GPU.\n");
  else
    {
      cudaDeviceProp prop;
      for (int i = 0; i < deviceCount; i++)
        {
          cudaGetDeviceProperties (&prop, i);
          printf ("Dev #%i is %s\n", i, prop.name);

        printf(" --- General Information for device %d ---\n", i);
	        printf("Name: %s\n", prop.name);
	        printf("Compute capability: %d.%d\n", prop.major, prop.minor);
	        printf("Clock rate: %d\n", prop.clockRate);
	        printf("Device copy overlap: ");
	        if (prop.deviceOverlap)
	            printf("Enabled\n");
	        else
	            printf("Disabled\n");
	        printf("Kernel execition timeout : ");
	        if (prop.kernelExecTimeoutEnabled)
	            printf("Enabled\n");
	        else
	            printf("Disabled\n");
	        printf(" --- Memory Information for device %d ---\n", i);
	        printf("Total global mem: %ld\n", prop.totalGlobalMem);
	        printf("Total constant Mem: %ld\n", prop.totalConstMem);
	        printf("Max mem pitch: %ld\n", prop.memPitch);
	        printf("Texture Alignment: %ld\n", prop.textureAlignment);
	        printf(" --- MP Information for device %d ---\n", i);
	        printf("Multiprocessor count: %d\n",
	               prop.multiProcessorCount);
	        printf("Shared mem per mp: %ld\n", prop.sharedMemPerBlock);
	        printf("Registers per mp: %d\n", prop.regsPerBlock);
	        printf("Threads in warp: %d\n", prop.warpSize);
	        printf("Max threads per block: %d\n",
              prop.maxThreadsPerBlock);
	        printf("Max thread dimensions: (%d, %d, %d)\n",
	               prop.maxThreadsDim[0], prop.maxThreadsDim[1],
	               prop.maxThreadsDim[2]);
	        printf("Max grid dimensions: (%d, %d, %d)\n",
	               prop.maxGridSize[0], prop.maxGridSize[1],
	               prop.maxGridSize[2]);



        }
    }
  return 1;
}
