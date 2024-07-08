#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Function to generate an array of unique random numbers
void generate_unique_random(int *arr, int N) {
    int i, j, is_unique;
    for (i = 0; i < N; i++) {
        do {
            arr[i] = rand() % (N * 10); // Generate random number
            is_unique = 1;
            // Check uniqueness with previous elements
            for (j = 0; j < i; j++) {
                if (arr[j] == arr[i]) {
                    is_unique = 0;
                    break;
                }
            }
        } while (!is_unique);
    }
}

// Function to perform rank sort serially
void rank_sort_serial(int *v, int *results, int N) {
    int i, j, rank[N];
    
    // Initialize ranks to 0
    for (i = 0; i < N; i++) {
        rank[i] = 0;
    }

    // Calculate ranks
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            if (v[i] > v[j]) {
                rank[i]++;
            }
        }
    }

    // Place elements in results array based on ranks
    for (i = 0; i < N; i++) {
        results[rank[i]] = v[i];
    }
}

// Function to perform rank sort in parallel using OpenMP for GPU
void rank_sort_parallel(int *v, int *results, int N, int num_threads, int num_blocks) {
    int i, j, rank[N];
    // num_threads=num_threads*num_blocks;
    // Initialize ranks to 0
    #pragma omp target teams distribute parallel for num_threads(num_threads) map(tofrom:rank)
    for (i = 0; i < N; i++) {
        rank[i] = 0;
    }

    // Calculate ranks in parallel
    #pragma omp target teams distribute parallel for collapse(2) num_threads(num_threads) map(to:v[:N], rank[:N])
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            if (v[i] > v[j]) {
                rank[i]++;
            }
        }
    }

    // Place elements in results array based on ranks in parallel
    #pragma omp target teams distribute parallel for num_threads(num_threads) map(to:v[:N], rank[:N], results[:N])
    for (i = 0; i < N; i++) {
        results[rank[i]] = v[i];
    }
}

int main() {
    int N, num_threads, num_blocks;
    printf("Enter the number of elements (N): ");
    scanf("%d", &N);
    printf("Enter the number of threads: ");
    scanf("%d", &num_threads);
    printf("Enter the number of blocks: ");
    scanf("%d", &num_blocks);

    // Allocate memory for arrays
    int *v = (int *)malloc(N * sizeof(int));
    int *results_cpu = (int *)malloc(N * sizeof(int));
    int *results_gpu = (int *)malloc(N * sizeof(int));

    // Generate random unique numbers
    generate_unique_random(v, N);

    // Print original array
    printf("Original Array:\n");
    for (int i = 0; i < N; i++) {
        printf("%d ", v[i]);
    }
    printf("\n");

    // Perform rank sort serially (on CPU)
    rank_sort_serial(v, results_cpu, N);

    // Print sorted array on CPU
    printf("Sorted Array (CPU):\n");
    for (int i = 0; i < N; i++) {
        printf("%d ", results_cpu[i]);
    }
    printf("\n");

    // Perform rank sort in parallel (on GPU)
    rank_sort_parallel(v, results_gpu, N, num_threads, num_blocks);

    // Print sorted array on GPU
    printf("Sorted Array (GPU):\n");
    for (int i = 0; i < N; i++) {
        printf("%d ", results_gpu[i]);
    }
    printf("\n");

    // Calculate speedup
    double start_cpu = omp_get_wtime();
    rank_sort_serial(v, results_cpu, N);
    double end_cpu = omp_get_wtime();

    double start_gpu = omp_get_wtime();
    rank_sort_parallel(v, results_gpu, N, num_threads, num_blocks);
    double end_gpu = omp_get_wtime();

    double time_cpu = end_cpu - start_cpu;
    double time_gpu = end_gpu - start_gpu;
    double speedup = time_cpu / time_gpu;

    printf("CPU Time: %.9f seconds\n", time_cpu);
    printf("GPU Time: %.9f seconds\n", time_gpu);
    printf("Speedup (CPU vs. GPU): %.9fx\n", speedup);

    // Free allocated memory
    free(v);
    free(results_cpu);
    free(results_gpu);

    return 0;
}
