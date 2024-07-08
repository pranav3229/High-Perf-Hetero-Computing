#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>	// for log function
#include <time.h>
#include <pthread.h>

int *arr, *new_arr;
int num_threads;
int n;
int i;

void* task(void* data)
{
	int start = *(int*)data;
	int len = *((int*)data + 1);
	free(data);

	for (int j = start; j < start + len; ++j)
		if (j < (1ll << i))
			new_arr[j] = arr[j];
		else
			new_arr[j] = arr[j] + arr[j - (1ll << i)];
}

void hillis_prefix()
{
	int q = n / num_threads;
	int r = n - q * num_threads;
	
	pthread_t* threads = calloc(num_threads, sizeof(pthread_t));
	
	for (i = 0; i < ceil(log2(n)); ++i)
	{
		for (int j = 0; j < num_threads; ++j)
		{
			int *data = calloc(2, sizeof(int));
			
			// initialise start and end index
			data[0] = (j <= r) ? (j * (q + 1)) : (j * q + r);
			data[1] = j < r ? q + 1 : q;

			// perform the operation
			pthread_create(threads + j, NULL, &task, (void*)data);
		}

		for (int j = 0; j < num_threads; ++j)
			pthread_join(threads[j], NULL);

		// swap tmp and arr
		int* tmp = arr;
		arr = new_arr;
		new_arr = tmp;
	}
	
	free(threads);
}

int main(int argc, char** argv)
{
	if (argc != 4)
	{
		printf("usage: ./hillis.o <input_file> <output_file> <num_threads>");
		exit(-1);
	}

	// change the input from stdin to file
	if (freopen(argv[1], "r", stdin) == NULL)
	{
		perror("error opening input file");
		exit(-1);
	}

	scanf("%d", &n);
	
	arr = calloc(n, sizeof(int));
	new_arr = calloc(n, sizeof(int));

	for (int i = 0; i < n; ++i)
		scanf("%d", &arr[i]);
	
	num_threads = atoi(argv[3]);

	clock_t start = clock();
	hillis_prefix();
	clock_t end = clock();
	
	double secs = (double)(end - start);
	printf("Time taken for prefix sum of %d elements: %f seconds.\n", n, secs / CLOCKS_PER_SEC);

	if (freopen(argv[2], "w", stdout) == NULL)
	{
		perror("writing to file");
		exit(-1);
	}

	printf("%d\n", n);
	for (int i = 0; i < n; ++i)
		printf("%d ", arr[i]);
	printf("\n");

	free(arr);
	free(new_arr);

	return 0;
}
