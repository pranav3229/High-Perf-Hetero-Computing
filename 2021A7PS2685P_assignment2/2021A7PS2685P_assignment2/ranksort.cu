#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// CUDA runtime
#include <cuda_runtime.h>

// Kernel function to perform rank sort on GPU
__global__ void rankSortKernel(int *input, int *output, int N) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;

    if (tid < N) {
        int rank = 0;
        for (int j = 0; j < N; ++j) {
            if (input[tid] > input[j]) {
                rank++;
            }
        }
        output[rank] = input[tid];
    }
}

// Function to perform rank sort on CPU
void rankSortCPU(int *input, int *output, int N) {
    for (int i = 0; i < N; ++i) {
        int rank = 0;
        for (int j = 0; j < N; ++j) {
            if (input[i] > input[j]) {
                rank++;
            }
        }
        output[rank] = input[i];
    }
}

// Function to shuffle an array using Fisher-Yates algorithm
void shuffleArray(int *array, int n) {
    srand(time(NULL));
    for (int i = n - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        // Swap array[i] and array[j]
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

int main() {
    int N;
    printf("Enter size of array (N): ");
    scanf("%d", &N);

    // Generate unique random numbers
    int *h_A = (int *)malloc(N * sizeof(int));
    int *uniqueNumbers = (int *)malloc(N * sizeof(int));
    srand(time(NULL));
    int index = 0;

    while (index < N) {
        int randomNum = rand() % 100000; // Generate random numbers between 0 and 99
        int isUnique = 1;
        // Check if randomNum is unique
        for (int i = 0; i < index; ++i) {
            if (uniqueNumbers[i] == randomNum) {
                isUnique = 0;
                break;
            }
        }
        if (isUnique) {
            uniqueNumbers[index++] = randomNum;
        }
    }

    // Shuffle the unique numbers
    shuffleArray(uniqueNumbers, N);

    // Copy shuffled unique numbers to h_A
    for (int i = 0; i < N; ++i) {
        h_A[i] = uniqueNumbers[i];
    }

    // Print shuffled array
    printf("Randomized & Shuffled Array:\n");
    for (int i = 0; i < N; ++i) {
        printf("%d ", h_A[i]);
    }
    printf("\n");

    // Allocate memory on GPU
    int *d_A, *d_B;
    cudaMalloc(&d_A, N * sizeof(int));
    cudaMalloc(&d_B, N * sizeof(int));

    // Copy input array from host to device
    cudaMemcpy(d_A, h_A, N * sizeof(int), cudaMemcpyHostToDevice);

    // Determine block and grid dimensions
    int T, B;
    printf("Enter number of threads per block (T): ");
    scanf("%d", &T);
    printf("Enter number of blocks in grid (B): ");
    scanf("%d", &B);

    // Ensure that T * B is less than N
    if (T * B < N) {
        printf("Error: Number of threads * blocks must be >= number of elements in the array (N)\n");
        return 1;
    }

    // Launch GPU kernel and time GPU execution
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);

    rankSortKernel<<<B, T>>>(d_A, d_B, N);

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    float gpuTime = 0;
    cudaEventElapsedTime(&gpuTime, start, stop);

    // Copy sorted array from device to host
    int *h_B = (int *)malloc(N * sizeof(int));
    cudaMemcpy(h_B, d_B, N * sizeof(int), cudaMemcpyDeviceToHost);

    // Perform rank sort on CPU and time CPU execution
    int *h_C = (int *)malloc(N * sizeof(int));
    clock_t cpuStart, cpuEnd;
    cpuStart = clock();
    rankSortCPU(h_A, h_C, N);
    cpuEnd = clock();
    float cpuTime = ((float)(cpuEnd - cpuStart)) / CLOCKS_PER_SEC * 1000;

    // Display results and timing
    printf("GPU Sorted Array:\n");
    for (int i = 0; i < N; ++i) {
        printf("%d ", h_B[i]);
    }
    printf("\n");
    printf("CPU Sorted Array:\n");
    for (int i = 0; i < N; ++i) {
        printf("%d ", h_C[i]);
    }
    printf("\n");

    printf("GPU Execution Time: %.8f ms\n", gpuTime);
    printf("CPU Execution Time: %.8f ms\n", cpuTime);

    // Compute and display speed-up factor
    float speedUp = cpuTime / gpuTime;
    printf("Speed-Up Factor (CPU vs GPU): %.8f\n", speedUp);

    // Clean up
    cudaFree(d_A);
    cudaFree(d_B);
    free(h_A);
    free(h_B);
    free(h_C);
    free(uniqueNumbers);

    return 0;
}
