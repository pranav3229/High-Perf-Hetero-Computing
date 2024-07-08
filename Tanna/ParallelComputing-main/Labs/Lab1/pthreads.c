#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define min(X,Y) ((X) > (Y) ? (Y) : (X))

double res;
double x;
pthread_mutex_t result_lock;

// calculate x^n using binary exponentiation algo
double binPow(double x, int n)
{
	double res = 1;

	while (n > 0)
	{
		if (n & 1)
			res = res * x;

		x = x * x;
		n >>= 1;
	}

	return res;
}

double factorial(int n)
{
	double res = 1;
	for (int i = 2; i <= n; ++i)
		res *= i;
	return res;
}

void* thread_work(void* data)
{
	int* arr = data;
	int pow_from = arr[0];
	int pow_to = arr[1];

	double fact = factorial(pow_from);
	double x_pow = binPow(x, pow_from);

	for (int i = pow_from; i < pow_to; ++i)
	{
		pthread_mutex_lock(&result_lock);
		res += x_pow / fact;
		pthread_mutex_unlock(&result_lock);

		x_pow *= x;
		fact *= (i + 1);
	}
	
	pthread_exit(0);
}

double e(int x)
{
	double res = 1;
	double fact = 1;
	double pw = 1;

	for (int i = 1; i < 101; ++i)
	{
		fact *= i;
		pw *= x;
		res += pw / fact;
	}
	return res;
}

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		printf("Invalid number of arguments\n");
		exit(-1);
	}

	x = strtod(argv[1], NULL);
	int N = atoi(argv[2]);
	
	pthread_t p_threads[N];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	
	res = 0;
	// sum will be calculated upto 100th term of taylor expansion
	
	clock_t start_time, end_time;
	start_time = clock();

	int* data_ptrs[N];
	for (int j = 1; j <= N; ++j)
	{
		int from = (j - 1) * (100 / N);
		int to = j * (100 / N);
		if (j == N)
			to = 101;

		data_ptrs[j - 1] = calloc(3, sizeof(int));
		data_ptrs[j - 1][0] = from;
		data_ptrs[j - 1][1] = to;
		data_ptrs[j - 1][2] = j;
	
		pthread_create(&p_threads[j - 1], &attr, thread_work, (void*)(data_ptrs[j - 1]));
	}

	for (int j = 0; j < N; ++j)
	{
		pthread_join(p_threads[j], NULL);
		free(data_ptrs[j]);
	}
	
	end_time = clock();
	double Tp = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

	start_time = clock();
	double res_s = e(x);
	end_time = clock();

	double Ts = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

	printf("result: %.15lf\n", res);
	printf("Speedup: %f\n", Ts / Tp);
	printf("Efficiency: %f\n", Ts / (N * Tp));

	return 0;
}
