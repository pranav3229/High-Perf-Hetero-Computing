#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>

#define NUM_THREADS 1000
#include "linearbarrier_pthreads.h"
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int total;
} linear_barrier_t;

void linear_barrier_init(linear_barrier_t *barrier, int total) {
    pthread_mutex_init(&barrier->mutex, NULL);
    pthread_cond_init(&barrier->cond, NULL);
    barrier->count = 0;
    barrier->total = total;
}

void linear_barrier_wait(linear_barrier_t *barrier) {
    pthread_mutex_lock(&barrier->mutex);
    barrier->count++;
    if (barrier->count < barrier->total) {
        pthread_cond_wait(&barrier->cond, &barrier->mutex);
    } else {
        barrier->count = 0;
        pthread_cond_broadcast(&barrier->cond);
    }
    pthread_mutex_unlock(&barrier->mutex);
}

void linear_barrier_destroy(linear_barrier_t *barrier) {
    pthread_mutex_destroy(&barrier->mutex);
    pthread_cond_destroy(&barrier->cond);
}

linear_barrier_t linear_barrier;

void* linear_barrier_test(void* arg) {
    linear_barrier_wait(&linear_barrier);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    struct timeval start, end;

    linear_barrier_init(&linear_barrier, NUM_THREADS);

    gettimeofday(&start, NULL);

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], NULL, linear_barrier_test, NULL);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end, NULL);

    linear_barrier_destroy(&linear_barrier);

    long long elapsed = (end.tv_sec - start.tv_sec) * 1000000LL + end.tv_usec - start.tv_usec;
    printf("Time taken by linear barrier: %lld microseconds\n", elapsed);

    return 0;
}
