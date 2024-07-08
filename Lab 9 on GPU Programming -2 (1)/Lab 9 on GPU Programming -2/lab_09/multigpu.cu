#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cuda.h>
__global__ void doSmt(int *data)
{
  // Simplification of above 
  int myID = ( blockIdx.z * gridDim.x * gridDim.y  + 
               blockIdx.y * gridDim.x + 
               blockIdx.x ) * blockDim.x + 
               threadIdx.x; 

  printf ("Hello world from %i\n", myID);
}

const int DATASIZE=1024;

int main ()
{

cudaStream_t str[4];
  int *h_data[4], *d_data[4];
  

  int deviceCount = 0;
  cudaGetDeviceCount (&deviceCount);
  if (deviceCount == 0)
    printf ("No CUDA compatible GPU.\n");
	  else
	    {     
	      for (int i = 0; i < deviceCount; i++)
	        {
				cudaSetDevice(i);
				printf("Allocating memory on GPU %d\n", i);
	           cudaStreamCreate(&(str[i]));
				h_data[i] = (int *)malloc(sizeof(int) * DATASIZE);
				cudaMalloc((void ** )&(d_data[i]), sizeof(int) * DATASIZE);
	
				// inititalize h_data[i]....
	
				printf("Trasferring data to memory on GPU %d\n", i);
	
				cudaMemcpyAsync(d_data[i], h_data[i], sizeof(int) * DATASIZE, cudaMemcpyHostToDevice, str[i]);
			   
				doSmt <<< 10, 256, 0, str[i] >>> (d_data[i]);
				
				printf("Trasferring data to host memory from GPU %d\n", i);
				cudaMemcpyAsync(h_data[i], d_data[i], sizeof(int) * DATASIZE, cudaMemcpyDeviceToHost, str[i]);    
	        }
	    }
	  return 1;
	}
