#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <sys/time.h>

double MPI_Sum(long my_rank, long comm_sz, long long n) {
    double sum = 0.0;
    double factor;
    long long i;
    long long my_n = n / comm_sz; // Number of terms for each process
    long long my_first_i = my_n * my_rank;
    long long my_last_i = my_first_i + my_n;

    if (my_rank == comm_sz - 1) { // Last process might have more terms
        my_last_i = n;
    }

    if (my_first_i % 2 == 0)
        factor = 1.0;
    else
        factor = -1.0;

    for (i = my_first_i; i < my_last_i; i++, factor = -factor) {
        sum += factor / (2 * i + 1);
    }

    return sum;
}

int main(int argc, char* argv[]) {
    int my_rank, comm_sz;
    long long n = 0;
    double global_sum, local_sum, piby4;
    double start_time, end_time;
    
    MPI_Init(&argc, &argv); // Initialize MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // Get process rank
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); // Get total number of processes
    
    if (argc != 3) {
        if (my_rank == 0) {
            printf("Usage: mpirun -np <number of processes> %s <number of terms> <number of processes>\n", argv[0]);
        }
        MPI_Finalize();
        exit(1);
    }

    n = atoll(argv[1]);
    long processes = atol(argv[2]);

    if (n % processes != 0) {
        if (my_rank == 0) {
            printf("Warning: Number of terms is not evenly divisible by the number of processes\n");
        }
    }

    start_time = MPI_Wtime(); // Start timing

    local_sum = MPI_Sum(my_rank, comm_sz, n);

    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    end_time = MPI_Wtime(); // End timing

    double parallel_time = end_time - start_time;

    // Calculate serial execution time on one process
    if (my_rank == 0) {
        start_time = MPI_Wtime(); // Start timing
        double serial_sum = MPI_Sum(0, 1, n);
        end_time = MPI_Wtime(); // End timing
    }
    double serial_time;
    MPI_Bcast(&serial_time, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        piby4 = global_sum;
        printf("Approximation of pi/4: %.16f\n", piby4);
        printf("Series: ");
        for (long long i = 0; i < n; i++) {
            if (i % 2 == 0)
                printf("+ 1/%lld ", 2 * i + 1);
            else
                printf("- 1/%lld ", 2 * i + 1);
        }
        printf("\n");

        serial_time = end_time - start_time;
        printf("Time taken with %ld processes: %.16lf seconds\n", processes, parallel_time);
        printf("Speedup: %.16lf\n", serial_time / parallel_time);
        printf("Efficiency: %.16lf\n", (serial_time / parallel_time) / processes);
    }

    MPI_Finalize(); // Finalize MPI
    return 0;
}
