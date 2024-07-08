#ifndef LINEAR_BARRIER_PTHREADS_H
#define LINEAR_BARRIER_PTHREADS_H

#include <pthread.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int total;
} linear_barrier_t;

void linear_barrier_init(linear_barrier_t *barrier, int total);
void linear_barrier_wait(linear_barrier_t *barrier);
void linear_barrier_destroy(linear_barrier_t *barrier);


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


#endif /* LINEAR_BARRIER_PTHREADS_H */
