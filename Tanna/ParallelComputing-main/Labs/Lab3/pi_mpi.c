#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main(int argc, char** argv)
{
    // MPI Variables
    int rank, nprocs;
    double start_time, end_time;

    // Initialise MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (argc != 2)
    {
        if (rank == 0)
            printf("Usage: ./a.out <number of terms>\n");
        exit(-1);
    }

    if (rank == 0)
        start_time = MPI_Wtime();

    int terms = atoi(argv[1]);
    int q = terms / nprocs;
    int r = terms % nprocs;
    int start = rank <= r ? (rank * (q + 1)) : (rank * q + r);
    int size = q + (rank < r);
    
    // Locally calculate the sum
    double sum = 0;
    for (int i = start; i < start + size; ++i)
        sum += (i % 2 ? -1.0 : 1.0) / (2.0 * i + 1.0);

    double global_sum = 0;

    // accumulate sum to rank 0
    MPI_Reduce(&sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0)
    {
        end_time = MPI_Wtime();
        double T1_start = MPI_Wtime();
        sum = 0;
        for (int i = 0; i < terms; ++i)
            sum += (i % 2 ? -1.0 : 1.0) / (2.0 * i + 1.0);
        double T1_end = MPI_Wtime();

        double T1 = T1_end - T1_start;
        double Tp = end_time - start_time;
        double speedup = T1 / Tp;
        double efficiency = speedup / nprocs;

        printf("pi: %f, speedup: %f, efficiency: %f\n", global_sum * 4, speedup, efficiency);
    }


    // end the program
    MPI_Finalize();
    return 0;
}