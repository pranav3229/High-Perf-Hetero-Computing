/*******************************************************************************
 * Aakash - 2018B4A70887P
 * The codes for linear and log barriers are taken from the book
 *
 *******************************************************************************/

#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

typedef struct linear_barrier
{
	pthread_mutex_t count_lock;
	pthread_cond_t ok_to_proceed;
	int count;
} linear_barrier;

typedef struct log_barrier
{
	pthread_mutex_t count_lock;
	pthread_cond_t ok_to_proceed_up;
	pthread_cond_t ok_to_proceed_down;
	int count;
} log_barrier;

int num_threads;
linear_barrier lin_bar;
log_barrier **log_bar;
pthread_mutex_t time_lock;
double time_taken = 0;

void init_linear_barrier(linear_barrier *b)
{
	b->count = 0;

	pthread_mutex_init(&(b->count_lock), NULL);
	pthread_cond_init(&(b->ok_to_proceed), NULL);
}

void init_log_barrier(log_barrier **b)
{
	for (int i = 0; i < num_threads; ++i)
	{
		b[i]->count = 0;
		pthread_mutex_init(&b[i]->count_lock, NULL);
		pthread_cond_init(&b[i]->ok_to_proceed_up, NULL);
		pthread_cond_init(&b[i]->ok_to_proceed_down, NULL);
	}
}

void linear_barrier_call(linear_barrier* b)
{
	pthread_mutex_lock(&(b->count_lock));
	b->count++;

	if (b->count == num_threads)
	{
		b->count = 0;
		pthread_cond_broadcast(&(b->ok_to_proceed));
	}
	else
		while (pthread_cond_wait(&(b->ok_to_proceed), &(b->count_lock)) != 0)
		{
			// don't release the lock yet
		}

	pthread_mutex_unlock(&(b->count_lock));
}

void log_barrier_call(log_barrier **b, int thread_id)
{
	int i = 2;
	int base = 0;

	do {
		int index = base + thread_id / i;

		if (thread_id % i == 0)
		{
			pthread_mutex_lock(&b[index]->count_lock);
			b[index]->count++;
			
			while (b[index]->count < 2)
				pthread_cond_wait(&b[index]->ok_to_proceed_up, &b[index]->count_lock);
			
			pthread_mutex_unlock(&b[index]->count_lock);
		}
		else
		{
			pthread_mutex_lock(&b[index]->count_lock);
			b[index]->count++;
			
			if (b[index]->count == 2)
				pthread_cond_signal(&b[index]->ok_to_proceed_up);

			while (pthread_cond_wait(&b[index]->ok_to_proceed_down, &b[index]->count_lock) != 0);

			pthread_mutex_unlock(&b[index]->count_lock);
			break;
		}

		base += num_threads / i;
		i *= 2;

	} while (i <= num_threads);

	i >>= 1;

	while (i > 1)
	{
		base -= num_threads / i;
		int index = base + thread_id / i;

		pthread_mutex_lock(&b[index]->count_lock);
		b[index]->count = 0;
		pthread_cond_signal(&b[index]->ok_to_proceed_down);
		pthread_mutex_unlock(&b[index]->count_lock);

		i >>= 1;
	}
}

void* thread_linear_task(void* s)
{

	// simulate some expensive task of 1 seconds
	sleep(1);

	clock_t start_time = clock();
	// pass through the barrier

	linear_barrier_call(&lin_bar);

	clock_t end_time = clock();
	double local_time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

	pthread_mutex_lock(&time_lock);
	time_taken += local_time_taken;
	printf("Time taken for linear barrier (in sec): %f\n", local_time_taken);
	pthread_mutex_unlock(&time_lock);
}

void* thread_log_task(void* s)
{
	// simulate some expensive task of 1 seconds
	
	sleep(1);

	int thread_index = *(int*)s;

	clock_t start_time = clock();
	log_barrier_call(log_bar, thread_index);
	clock_t end_time = clock();

	double local_time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

	pthread_mutex_lock(&time_lock);
	time_taken += local_time_taken;

	printf("Time taken for log barrier (in sec): %f\n", local_time_taken);

	pthread_mutex_unlock(&time_lock);
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("usage: barrier.out <Num Threads>\n");
		exit(-1);
	}

	// initiaise threads to run
	num_threads = atoi(argv[1]);
	pthread_t *p_threads = calloc(num_threads, sizeof(pthread_t));
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	init_linear_barrier(&lin_bar);
	pthread_mutex_init(&time_lock, NULL);

	printf("Calling linear barrier for n = %d threads...\n", num_threads);
	for (int i = 0; i < num_threads; ++i)
		pthread_create(&p_threads[i], &attr, thread_linear_task, NULL);

	for (int i = 0; i < num_threads; ++i)
		pthread_join(p_threads[i], NULL);

	double lin_time = time_taken / num_threads;

	free(p_threads);

	// call for log barrier
	int *ids = calloc(num_threads, sizeof(int));
	for (int i = 0; i < num_threads; ++i)
		ids[i] = i;

	log_bar = calloc(num_threads, sizeof(void*));
	for (int i = 0; i < num_threads; ++i)
		log_bar[i] = calloc(1, sizeof(log_barrier));

	init_log_barrier(log_bar);
	time_taken = 0;

	p_threads = calloc(num_threads, sizeof(pthread_t));
	
	printf("Calling log barrier for n = %d threads...\n", num_threads);

	// call threads
	for (int i = 0; i < num_threads; ++i)
		pthread_create(&p_threads[i], &attr, thread_log_task, ids + i);

	// wait for all threads to finish
	for (int i = 0; i < num_threads; ++i)
		pthread_join(p_threads[i], NULL);

	// calculations
	double log_time = time_taken / num_threads;
	free(p_threads);
	for (int i = 0; i < num_threads; ++i)
		free(log_bar[i]);
	free(log_bar);
	free(ids);

	printf("Average time taken for linear barrier call (in sec): %f\n", lin_time);
	printf("Average time taken for log barrier call (in sec): %f\n", log_time);
}
