#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>


typedef struct {
    int** data;
    int size;
} Matrix;

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

Matrix addMatrices(Matrix A, Matrix B) {
    Matrix result = allocateMatrix(A.size);
    #pragma omp parallel for
    for (int i = 0; i < A.size; i++) {
        for (int j = 0; j < A.size; j++) {
            result.data[i][j] = A.data[i][j] + B.data[i][j];
        }
    }
    return result;
}


Matrix subtractMatrices(Matrix A, Matrix B) {
    Matrix result = allocateMatrix(A.size);
    #pragma omp parallel for
    for (int i = 0; i < A.size; i++) {
        for (int j = 0; j < A.size; j++) {
            result.data[i][j] = A.data[i][j] - B.data[i][j];
        }
    }
    return result;
}


void partitionMatrix(Matrix original, Matrix *A11, Matrix *A12, Matrix *A21, Matrix *A22) {
    int newSize = original.size / 2;
    #pragma omp parallel for
    for (int i = 0; i < newSize; i++) {
        for (int j = 0; j < newSize; j++) {
            A11->data[i][j] = original.data[i][j];
            A12->data[i][j] = original.data[i][j + newSize];
            A21->data[i][j] = original.data[i + newSize][j];
            A22->data[i][j] = original.data[i + newSize][j + newSize];
        }
    }
}


void combineMatrix(Matrix *result, Matrix C11, Matrix C12, Matrix C21, Matrix C22) {
    int newSize = result->size / 2;
    #pragma omp parallel for
    for (int i = 0; i < newSize; i++) {
        for (int j = 0; j < newSize; j++) {
            result->data[i][j] = C11.data[i][j];
            result->data[i][j + newSize] = C12.data[i][j];
            result->data[i + newSize][j] = C21.data[i][j];
            result->data[i + newSize][j + newSize] = C22.data[i][j];
        }
    }
}


// void* matrixOperation(void* arg) {
//     ThreadData* data = (ThreadData*)arg;

//     if (data->operation == 0) { // Addition
//         *(data->result) = addMatrices(data->A, data->B);
//     } else if (data->operation == 1) { // Subtraction
//         *(data->result) = subtractMatrices(data->A, data->B);
//     } else if (data->operation == 2) { // Multiplication
//         *(data->result) = strassenMultiply(data->A, data->B);
//     }
//     return NULL;
// }

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
Matrix strassenMultiply(Matrix A, Matrix B) {
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
        Matrix M1,M2,M3,M4,M5,M6,M7;
        Matrix C11, C12, C21, C22;

        #pragma omp parallel
        {

        

        
            
        // Assuming partitionMatrix is a function that partitions a matrix into its quadrants
        partitionMatrix(A, &A11, &A12, &A21, &A22);
        partitionMatrix(B, &B11, &B12, &B21, &B22);
        

        

        #pragma omp barrier
        

        
        M1 = strassenMultiply(addMatrices(A11, A22), addMatrices(B11, B22));
        M2 = strassenMultiply(addMatrices(A21, A22), B11);
        M3 = strassenMultiply(A11, subtractMatrices(B12, B22));
        M4 = strassenMultiply(A22, subtractMatrices(B21, B11));
        M5 = strassenMultiply(addMatrices(A11, A12), B22);
        M6 = strassenMultiply(subtractMatrices(A21, A11), addMatrices(B11, B12));
        M7 = strassenMultiply(subtractMatrices(A12, A22), addMatrices(B21, B22));
        

        
        #pragma omp barrier
        
        C11 = addMatrices(subtractMatrices(addMatrices(M1, M4), M5), M7);
        C12 = addMatrices(M3, M5);
        C21 = addMatrices(M2, M4);
        C22 = addMatrices(subtractMatrices(addMatrices(M1, M3), M2), M6);
        

        
        }
    
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
    Matrix C = strassenMultiply(A, B);
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
    Matrix C_parallel = strassenMultiply(A, B);
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