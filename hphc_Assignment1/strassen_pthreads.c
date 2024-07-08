#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    int** data;
    int size;
} Matrix;

typedef struct {
    Matrix A;
    Matrix B;
    Matrix *result;
    int operation; // 0 for add, 1 for subtract, 2 for multiply
} ThreadData;


Matrix strassenMultiply_parallel(Matrix A, Matrix B);


Matrix allocateMatrix(int n) {
    Matrix mat;
    mat.size = n;
    mat.data = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        mat.data[i] = (int*)malloc(n * sizeof(int));
    }
    return mat;
}

void freeMatrix(Matrix *mat) {
    for (int i = 0; i < mat->size; i++) {
        free(mat->data[i]);
    }
    free(mat->data);
    mat->size = 0;
}

void populateMatrix(Matrix *mat) {
    printf("Enter the elements for a %dx%d matrix:\n", mat->size, mat->size);
    for (int i = 0; i < mat->size; i++) {
        for (int j = 0; j < mat->size; j++) {
            scanf("%d", &mat->data[i][j]);
        }
    }
}

void printMatrix(Matrix *mat) {
    for (int i = 0; i < mat->size; i++) {
        for (int j = 0; j < mat->size; j++) {
            printf("%d\t", mat->data[i][j]);
        }
        printf("\n");
    }
}

typedef struct {
    int startRow;
    int endRow;
    Matrix *A;
    Matrix *B;
    Matrix *result;
} AddSubtractArgs;

void* addMatricesThread(void* args) {
    AddSubtractArgs* addArgs = (AddSubtractArgs*)args;
    for (int i = addArgs->startRow; i < addArgs->endRow; i++) {
        for (int j = 0; j < addArgs->A->size; j++) {
            addArgs->result->data[i][j] = addArgs->A->data[i][j] + addArgs->B->data[i][j];
        }
    }
    return NULL;
}

Matrix addMatricesParallel(Matrix A, Matrix B) {
    int nThreads = 1; 
    pthread_t threads[nThreads];
    AddSubtractArgs args[nThreads];
    Matrix result = allocateMatrix(A.size);

    int rowsPerThread = A.size / nThreads;
    for (int i = 0; i < nThreads; i++) {
        args[i].A = &A;
        args[i].B = &B;
        args[i].result = &result;
        args[i].startRow = i * rowsPerThread;
        args[i].endRow = (i + 1) * rowsPerThread;
        if (i == nThreads - 1) args[i].endRow = A.size; // Handle the remainder for the last thread
        pthread_create(&threads[i], NULL, addMatricesThread, &args[i]);
    }

    for (int i = 0; i < nThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    return result;
}


Matrix addMatrices(Matrix A, Matrix B) {
    Matrix result = allocateMatrix(A.size);
    for (int i = 0; i < A.size; i++) {
        for (int j = 0; j < A.size; j++) {
            result.data[i][j] = A.data[i][j] + B.data[i][j];
        }
    }
    return result;
}

void* subtractMatricesThread(void* args) {
    AddSubtractArgs* subtractArgs = (AddSubtractArgs*)args;
    for (int i = subtractArgs->startRow; i < subtractArgs->endRow; i++) {
        for (int j = 0; j < subtractArgs->A->size; j++) {
            subtractArgs->result->data[i][j] = subtractArgs->A->data[i][j] - subtractArgs->B->data[i][j];
        }
    }
    return NULL;
}

Matrix subtractMatricesParallel(Matrix A, Matrix B) {
    int nThreads = 1; 
    pthread_t threads[nThreads];
    AddSubtractArgs args[nThreads];
    Matrix result = allocateMatrix(A.size);

    int rowsPerThread = A.size / nThreads;
    for (int i = 0; i < nThreads; i++) {
        args[i].A = &A;
        args[i].B = &B;
        args[i].result = &result;
        args[i].startRow = i * rowsPerThread;
        args[i].endRow = (i + 1) * rowsPerThread;
        if (i == nThreads - 1) args[i].endRow = A.size; // Ensure the last thread covers the remainder

        pthread_create(&threads[i], NULL, subtractMatricesThread, &args[i]);
    }

    for (int i = 0; i < nThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    return result;
}


Matrix subtractMatrices(Matrix A, Matrix B) {
    Matrix result = allocateMatrix(A.size);
    for (int i = 0; i < A.size; i++) {
        for (int j = 0; j < A.size; j++) {
            result.data[i][j] = A.data[i][j] - B.data[i][j];
        }
    }
    return result;
}

typedef struct {
    Matrix original;
    Matrix *A11;
    Matrix *A12;
    Matrix *A21;
    Matrix *A22;
    int startRow;
    int endRow;
} PartitionArgs;

void* partitionMatrixThread(void* args) {
    PartitionArgs* pArgs = (PartitionArgs*)args;
    int newSize = pArgs->original.size / 2;
    for (int i = pArgs->startRow; i < pArgs->endRow; i++) {
        for (int j = 0; j < newSize; j++) {
            if (i < newSize) {
                pArgs->A11->data[i][j] = pArgs->original.data[i][j];
                pArgs->A12->data[i][j] = pArgs->original.data[i][j + newSize];
            } else {
                pArgs->A21->data[i - newSize][j] = pArgs->original.data[i][j];
                pArgs->A22->data[i - newSize][j] = pArgs->original.data[i][j + newSize];
            }
        }
    }
    return NULL;
}

void partitionMatrixParallel(Matrix original, Matrix *A11, Matrix *A12, Matrix *A21, Matrix *A22) {
    int nThreads = 4; 
    pthread_t threads[nThreads];
    PartitionArgs args[nThreads];
    int newSize = original.size / 2;
    int rowsPerThread = newSize / nThreads; // Dividing the work by the number of threads

    // Initialize matrices for sub-parts
    *A11 = allocateMatrix(newSize);
    *A12 = allocateMatrix(newSize);
    *A21 = allocateMatrix(newSize);
    *A22 = allocateMatrix(newSize);

    for (int i = 0; i < nThreads; i++) {
        args[i].original = original;
        args[i].A11 = A11;
        args[i].A12 = A12;
        args[i].A21 = A21;
        args[i].A22 = A22;
        args[i].startRow = i * rowsPerThread * 2; // Each thread works on a section of the original matrix
        args[i].endRow = (i + 1) * rowsPerThread * 2;
        if (i == nThreads - 1) args[i].endRow = original.size; // The last thread may have more work if not evenly divisible

        pthread_create(&threads[i], NULL, partitionMatrixThread, &args[i]);
    }

    for (int i = 0; i < nThreads; i++) {
        pthread_join(threads[i], NULL);
    }
}


void partitionMatrix(Matrix original, Matrix *A11, Matrix *A12, Matrix *A21, Matrix *A22) {
    int newSize = original.size / 2;
    for (int i = 0; i < newSize; i++) {
        for (int j = 0; j < newSize; j++) {
            A11->data[i][j] = original.data[i][j];
            A12->data[i][j] = original.data[i][j + newSize];
            A21->data[i][j] = original.data[i + newSize][j];
            A22->data[i][j] = original.data[i + newSize][j + newSize];
        }
    }
}

typedef struct {
    Matrix *result;
    Matrix C11;
    Matrix C12;
    Matrix C21;
    Matrix C22;
    int startRow;
    int endRow;
} CombineArgs;

void* combineMatrixThread(void* args) {
    CombineArgs* cArgs = (CombineArgs*)args;
    int newSize = cArgs->result->size / 2;
    for (int i = cArgs->startRow; i < cArgs->endRow; i++) {
        for (int j = 0; j < newSize; j++) {
            if (i < newSize) {
                cArgs->result->data[i][j] = cArgs->C11.data[i][j];
                cArgs->result->data[i][j + newSize] = cArgs->C12.data[i][j];
            } else {
                cArgs->result->data[i][j] = cArgs->C21.data[i - newSize][j];
                cArgs->result->data[i][j + newSize] = cArgs->C22.data[i - newSize][j];
            }
        }
    }
    return NULL;
}

void combineMatrixParallel(Matrix *result, Matrix C11, Matrix C12, Matrix C21, Matrix C22) {
    int nThreads = 1; 
    pthread_t threads[nThreads];
    CombineArgs args[nThreads];
    int newSize = result->size / 2;
    int rowsPerThread = newSize / nThreads; // Dividing the work by the number of threads

    for (int i = 0; i < nThreads; i++) {
        args[i].result = result;
        args[i].C11 = C11;
        args[i].C12 = C12;
        args[i].C21 = C21;
        args[i].C22 = C22;
        args[i].startRow = i * rowsPerThread * 2; // Each thread works on a section of the result matrix
        args[i].endRow = (i + 1) * rowsPerThread * 2;
        if (i == nThreads - 1) args[i].endRow = result->size; // The last thread may have more work if not evenly divisible

        pthread_create(&threads[i], NULL, combineMatrixThread, &args[i]);
    }

    for (int i = 0; i < nThreads; i++) {
        pthread_join(threads[i], NULL);
    }
}



void combineMatrix(Matrix *result, Matrix C11, Matrix C12, Matrix C21, Matrix C22) {
    int newSize = result->size / 2;
    for (int i = 0; i < newSize; i++) {
        for (int j = 0; j < newSize; j++) {
            result->data[i][j] = C11.data[i][j];
            result->data[i][j + newSize] = C12.data[i][j];
            result->data[i + newSize][j] = C21.data[i][j];
            result->data[i + newSize][j + newSize] = C22.data[i][j];
        }
    }
}
void* matrixOperation(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    if (data->operation == 0) { // Addition
        *(data->result) = addMatricesParallel(data->A, data->B);
    } else if (data->operation == 1) { // Subtraction
        *(data->result) = subtractMatricesParallel(data->A, data->B);
    } else if (data->operation == 2) { // Multiplication
        *(data->result) = strassenMultiply_parallel(data->A, data->B);
    }
    return NULL;
}

Matrix strassenMultiply_serial(Matrix A, Matrix B) {
    int n = A.size;
    Matrix C = allocateMatrix(n);

    if (n == 1) {
        C.data[0][0] = A.data[0][0] * B.data[0][0];
    } else {
        int newSize = n / 2;
        Matrix A11 = allocateMatrix(newSize);
        Matrix A12 = allocateMatrix(newSize);
        Matrix A21 = allocateMatrix(newSize);
        Matrix A22 = allocateMatrix(newSize);
        Matrix B11 = allocateMatrix(newSize);
        Matrix B12 = allocateMatrix(newSize);
        Matrix B21 = allocateMatrix(newSize);
        Matrix B22 = allocateMatrix(newSize);

        // Assuming partitionMatrix is a function that partitions a matrix into its quadrants
        partitionMatrix(A, &A11, &A12, &A21, &A22);
        partitionMatrix(B, &B11, &B12, &B21, &B22);

        Matrix M1 = strassenMultiply_serial(addMatrices(A11, A22), addMatrices(B11, B22));
        Matrix M2 = strassenMultiply_serial(addMatrices(A21, A22), B11);
        Matrix M3 = strassenMultiply_serial(A11, subtractMatrices(B12, B22));
        Matrix M4 = strassenMultiply_serial(A22, subtractMatrices(B21, B11));
        Matrix M5 = strassenMultiply_serial(addMatrices(A11, A12), B22);
        Matrix M6 = strassenMultiply_serial(subtractMatrices(A21, A11), addMatrices(B11, B12));
        Matrix M7 = strassenMultiply_serial(subtractMatrices(A12, A22), addMatrices(B21, B22));

        Matrix C11 = addMatrices(subtractMatrices(addMatrices(M1, M4), M5), M7);
        Matrix C12 = addMatrices(M3, M5);
        Matrix C21 = addMatrices(M2, M4);
        Matrix C22 = addMatrices(subtractMatrices(addMatrices(M1, M3), M2), M6);

        // Assuming combineMatrix is a function that combines four quadrants into one matrix
        combineMatrix(&C, C11, C12, C21, C22);

        // Free temporary matrices
        freeMatrix(&A11); freeMatrix(&A12); freeMatrix(&A21); freeMatrix(&A22);
        freeMatrix(&B11); freeMatrix(&B12); freeMatrix(&B21); freeMatrix(&B22);
        freeMatrix(&M1); freeMatrix(&M2); freeMatrix(&M3); freeMatrix(&M4);
        freeMatrix(&M5); freeMatrix(&M6); freeMatrix(&M7);
        freeMatrix(&C11); freeMatrix(&C12); freeMatrix(&C21); freeMatrix(&C22);
}

    return C;
}
Matrix strassenMultiply_parallel(Matrix A, Matrix B) {
    int n = A.size;
    if (n == 1) {
        Matrix C = allocateMatrix(1);
        C.data[0][0] = A.data[0][0] * B.data[0][0];
        return C;
    }

    int newSize = n / 2;
    Matrix C = allocateMatrix(n);
    Matrix A11 = allocateMatrix(newSize), A12 = allocateMatrix(newSize),
           A21 = allocateMatrix(newSize), A22 = allocateMatrix(newSize),
           B11 = allocateMatrix(newSize), B12 = allocateMatrix(newSize),
           B21 = allocateMatrix(newSize), B22 = allocateMatrix(newSize);

    partitionMatrix(A, &A11, &A12, &A21, &A22);
    partitionMatrix(B, &B11, &B12, &B21, &B22);

    pthread_t threads[7];
    ThreadData data[7];
    Matrix M[7]; // Temporary matrices for storing intermediate results
    for (int i = 0; i < 7; i++) {
        M[i] = allocateMatrix(newSize);
        data[i].result = &M[i];
        data[i].operation = 2; // Set operation to multiply
    }

    // Prepare thread data for M1 to M7 according to Strassen's algorithm
    // for M1:
    data[0].A = addMatrices(A11, A22);
    data[0].B = addMatrices(B11, B22);
    pthread_create(&threads[0], NULL, matrixOperation, &data[0]);
    
    // M2 = (A21 + A22) * B11
    data[1].A = addMatricesParallel(A21, A22);
    data[1].B = B11;
    pthread_create(&threads[1], NULL, matrixOperation, &data[1]);

    // M3 = A11 * (B12 - B22)
    data[2].A = A11;
    data[2].B = subtractMatricesParallel(B12, B22);
    pthread_create(&threads[2], NULL, matrixOperation, &data[2]);

    // M4 = A22 * (B21 - B11)
    data[3].A = A22;
    data[3].B = subtractMatricesParallel(B21, B11);
    pthread_create(&threads[3], NULL, matrixOperation, &data[3]);

    // M5 = (A11 + A12) * B22
    data[4].A = addMatricesParallel(A11, A12);
    data[4].B = B22;
    pthread_create(&threads[4], NULL, matrixOperation, &data[4]);

    // M6 = (A21 - A11) * (B11 + B12)
    data[5].A = subtractMatricesParallel(A21, A11);
    data[5].B = addMatricesParallel(B11, B12);
    pthread_create(&threads[5], NULL, matrixOperation, &data[5]);

    // M7 = (A12 - A22) * (B21 + B22)
    data[6].A = subtractMatricesParallel(A12, A22);
    data[6].B = addMatricesParallel(B21, B22);
    pthread_create(&threads[6], NULL, matrixOperation, &data[6]);

    // Wait for all threads to complete
    for (int i = 0; i < 7; i++) {
        pthread_join(threads[i], NULL);
    }

    // Construct C11, C12, C21, C22 using results from M1 to M7
    Matrix C11 = addMatricesParallel(subtractMatricesParallel(addMatricesParallel(M[0], M[3]), M[4]), M[6]);
    Matrix C12 = addMatricesParallel(M[2], M[4]);
    Matrix C21 = addMatricesParallel(M[1], M[3]);
    Matrix C22 = addMatricesParallel(subtractMatricesParallel(addMatricesParallel(M[0], M[2]), M[1]), M[5]);

    // Combine the results into the final matrix C
    combineMatrixParallel(&C, C11, C12, C21, C22);

    // Free all allocated memory
    freeMatrix(&A11); freeMatrix(&A12); freeMatrix(&A21); freeMatrix(&A22);
    freeMatrix(&B11); freeMatrix(&B12); freeMatrix(&B21); freeMatrix(&B22);
    for (int i = 0; i < 7; i++) {
        freeMatrix(&M[i]);
    }
    freeMatrix(&C11); freeMatrix(&C12); freeMatrix(&C21); freeMatrix(&C22);

    return C;
}



int main() {
    int n;
    printf("Enter the dimension of the matrices (n x n), where n is a power of 2: ");
    scanf("%d", &n);

    if (n & (n - 1)) {
        fprintf(stderr, "Dimension is not a power of 2.\n");
        return EXIT_FAILURE;
    }

    Matrix A = allocateMatrix(n);
    Matrix B = allocateMatrix(n);

    printf("Populate matrix A:\n");
    populateMatrix(&A);
    printf("Populate matrix B:\n");
    populateMatrix(&B);

    printMatrix(&A);
    printf("\n");
    printMatrix(&B);
    printf("\n");

    // Perform Strassen's multiplication
    Matrix C = strassenMultiply_parallel(A, B);
    printf("Result of Strassen's Multiplication:\n");
    printMatrix(&C);
    printf("\n");


        // Measure execution time for serial version
    clock_t start_serial = clock();
    Matrix C_serial = strassenMultiply_serial(A, B);
    clock_t end_serial = clock();
    double time_serial = (double)(end_serial - start_serial) / CLOCKS_PER_SEC;

    // Measure execution time for parallel version
    clock_t start_parallel = clock();
    Matrix C_parallel = strassenMultiply_parallel(A, B);
    clock_t end_parallel = clock();
    double time_parallel = (double)(end_parallel - start_parallel) / CLOCKS_PER_SEC;

    // Compute and print the speedup
    double speedup = time_serial / time_parallel;
    printf("Serial Time: %.8f\n",time_serial);
    printf("Parallel Time: %.8f\n",time_parallel);
    printf("Speedup: %.8f\n", speedup);

    // Free the matrices after use
    freeMatrix(&A);
    freeMatrix(&B);
    freeMatrix(&C);

    return 0;
}
