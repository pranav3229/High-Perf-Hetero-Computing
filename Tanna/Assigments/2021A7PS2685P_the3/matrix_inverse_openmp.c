#include <time.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>

void cofactor_parallel(float [][25], float);
float determinant_parallel(float [][25], float);
void transpose_parallel(float [][25], float [][25], float);

//function prototype that are being created
void cofactor(float [][25], float);
float determinant(float [][25], float);
void transpose(float [][25], float [][25], float);

 
// function for the calculation of determinant
float determinant(float a[25][25], float k)
{
  float s = 1, det = 0, b[25][25];
  int i, j, m, n, c;
  if (k == 1)
    {
     return (a[0][0]);
    }
  else
    {
     det = 0;
     for (c = 0; c < k; c++)
       {
        m = 0;
        n = 0;
        for (i = 0;i < k; i++)
          {
            for (j = 0 ;j < k; j++)
              {
                b[i][j] = 0;
                if (i != 0 && j != c)
                 {
                   b[m][n] = a[i][j];
                   if (n < (k - 2))
                    n++;
                   else
                    {
                     n = 0;
                     m++;
                     }
                   }
               }
             }
          det = det + s * (a[0][c] * determinant(b, k - 1));
          s = -1 * s;
          }
    }
 
    return (det);
}
 
 
// function for cofactor calculation
void cofactor(float num[25][25], float f)
{
 float b[25][25], fac[25][25];
 int p, q, m, n, i, j;
 for (q = 0;q < f; q++)
 {
   for (p = 0;p < f; p++)
    {
     m = 0;
     n = 0;
     for (i = 0;i < f; i++)
     {
       for (j = 0;j < f; j++)
        {
          if (i != q && j != p)
          {
            b[m][n] = num[i][j];
            if (n < (f - 2))
             n++;
            else
             {
               n = 0;
               m++;
               }
            }
        }
      }
      fac[q][p] = pow(-1, q + p) * determinant(b, f - 1);
    }
  }
  transpose(num, fac, f);
}
 
 
///function to find the transpose of a matrix
void transpose(float num[25][25], float fac[25][25], float r)
{
  int i, j;
  float b[25][25], inverse[25][25], d;
 
  for (i = 0;i < r; i++)
    {
     for (j = 0;j < r; j++)
       {
         b[i][j] = fac[j][i];
        }
    }
    
  d = determinant(num, r);
  for (i = 0;i < r; i++)
    {
     for (j = 0;j < r; j++)
       {
        inverse[i][j] = b[i][j] / d;
        }
    }
    
   printf("\nThe inverse of matrix: \n");
   for (i = 0;i < r; i++)
    {
     for (j = 0;j < r; j++)
       {
         printf("\t%f", inverse[i][j]);
        }
    printf("\n");
     }
}

float determinant_parallel(float a[25][25], float k) {
    float s = 1, det = 0, b[25][25];
    int i, j, m, n, c;

    if (k == 1) {
        return (a[0][0]);
    } else {
        det = 0;
#pragma omp parallel for private(b, m, n, i, j) reduction(+:det) shared(s) 
        for (c = 0; c < (int)k; c++) {
            m = 0;
            n = 0;
            for (i = 0; i < k; i++) {
                for (j = 0; j < k; j++) {
                    b[i][j] = 0;
                    if (i != 0 && j != c) {
                        b[m][n] = a[i][j];
                        if (n < (k - 2))
                            n++;
                        else {
                            n = 0;
                            m++;
                        }
                    }
                }
            }
            det += s * (a[0][c] * determinant_parallel(b, k - 1));
            s = -s;
        }
    }

    return det;
}

void cofactor_parallel(float num[25][25], float f) {
    float b[25][25], fac[25][25];
    int p, q, m, n, i, j;

#pragma omp parallel for private (m,n,p,i,j,b) shared(fac, num, q) // Remove 'b', 'fac' from private clause
    for (q = 0; q < (int)f; q++) {
#pragma omp parallel for private(p, m, n, i, j, b) shared(fac,num,q) // Remove 'm', 'n' from private clause
        for (p = 0; p < (int)f; p++) {
            m = 0;
            n = 0;
            for (i = 0; i < f; i++) {
                for (j = 0; j < f; j++) {
                    if (i != q && j != p) {
                        b[m][n] = num[i][j];
                        if (n < (f - 2))
                            n++;
                        else {
                            n = 0;
                            m++;
                        }
                    }
                }
            }
            fac[q][p] = pow(-1, q + p) * determinant_parallel(b, f - 1);
        }
    }

    transpose_parallel(num, fac, f);
}

void transpose_parallel(float num[25][25], float fac[25][25], float r) {
    int i, j;
    float b[25][25], inverse[25][25], d;

#pragma omp parallel for private(i, j) // Remove 'b', 'inverse', 'd' from private clause
    for (i = 0; i < (int)r; i++) {
#pragma omp parallel for private(j) // Remove 'j' from private clause
        for (j = 0; j < (int)r; j++) {
            b[i][j] = fac[j][i];
        }
    }

    d = determinant_parallel(num, r);

#pragma omp parallel for private(i, j) // Remove 'i', 'j' from private clause
    for (i = 0; i < (int)r; i++) {
#pragma omp parallel for private(j) // Remove 'j' from private clause
        for (j = 0; j < (int)r; j++) {
            inverse[i][j] = b[i][j] / d;
        }
    }

    printf("\nThe inverse of matrix: \n");
    for (i = 0; i < r; i++) {
        for (j = 0; j < r; j++) {
            printf("\t%f", inverse[i][j]);
        }
        printf("\n");
    }
}

void serial_call() {
    float a[25][25], n, d;
    int i, j;

    printf("Enter the order of the Matrix: ");
    scanf("%f", &n);
    printf("Enter the elements of a matrix: \n");
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            scanf("%f", &a[i][j]);
        }
    }

    d = determinant(a, n);
    if (d == 0)
        printf("Since the determinant is zero (0), therefore inverse is not possible.");
    else
        cofactor(a, n);
}

void parallel_call() {
    float a[25][25], n, d;
    int i, j;

    printf("Enter the order of the Matrix: ");
    scanf("%f", &n);
    printf("Enter the elements of a matrix: \n");
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            scanf("%f", &a[i][j]);
        }
    }

    d = determinant_parallel(a, n);
    printf("Determinant: %f\n", d);
    if (d == 0)
        printf("Since the determinant is zero (0), therefore inverse is not possible.");
    else
        cofactor_parallel(a, n);
}

int main() {
    clock_t start, end;
    double cpu_time_used;

    // Serial execution
    start = clock();
    serial_call();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Serial Execution Time: %f seconds\n", cpu_time_used);

    // Parallel execution
    start = clock();
    parallel_call();
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Parallel Execution Time: %f seconds\n", cpu_time_used);

    return 0;
}
