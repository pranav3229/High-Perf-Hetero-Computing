#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "linearbarrier_pthreads.h"
#include "treebarrier_pthreads.h"

#define NUM_THREADS 1000

void* linear_barrier_test(void* arg) {
    linear_barrier_wait((linear_barrier_t*)arg);
    return NULL;
}

void* tree_barrier_test(void* arg) {
    tree_barrier_wait((tree_barrier_t*)arg);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    struct timeval start, end;
    long long elapsed;

    // Test with 10 threads
    linear_barrier_t linear_barrier_10;
    linear_barrier_init(&linear_barrier_10, 10);

    tree_barrier_t tree_barrier_10;
    tree_barrier_init(&tree_barrier_10, 10);

    gettimeofday(&start, NULL);
    for (int i = 0; i < 10; ++i) {
        pthread_create(&threads[i], NULL, linear_barrier_test, &linear_barrier_10);
    }
    for (int i = 0; i < 10; ++i) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end, NULL);
    elapsed = (end.tv_sec - start.tv_sec) * 1000000LL + end.tv_usec - start.tv_usec;
    printf("Time taken by linear barrier with 10 threads: %lld microseconds\n", elapsed);

    gettimeofday(&start, NULL);
    for (int i = 0; i < 10; ++i) {
        pthread_create(&threads[i], NULL, tree_barrier_test, &tree_barrier_10);
    }
    for (int i = 0; i < 10; ++i) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end, NULL);
    elapsed = (end.tv_sec - start.tv_sec) * 1000000LL + end.tv_usec - start.tv_usec;
    printf("Time taken by tree barrier with 10 threads: %lld microseconds\n", elapsed);

    linear_barrier_destroy(&linear_barrier_10);
    tree_barrier_destroy(&tree_barrier_10);

    // Test with 100 threads
    linear_barrier_t linear_barrier_100;
    linear_barrier_init(&linear_barrier_100, 100);

    tree_barrier_t tree_barrier_100;
    tree_barrier_init(&tree_barrier_100, 100);

    gettimeofday(&start, NULL);
    for (int i = 0; i < 100; ++i) {
        pthread_create(&threads[i], NULL, linear_barrier_test, &linear_barrier_100);
    }
    for (int i = 0; i < 100; ++i) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end, NULL);
    elapsed = (end.tv_sec - start.tv_sec) * 1000000LL + end.tv_usec - start.tv_usec;
    printf("Time taken by linear barrier with 100 threads: %lld microseconds\n", elapsed);

    gettimeofday(&start, NULL);
    for (int i = 0; i < 100; ++i) {
        pthread_create(&threads[i], NULL, tree_barrier_test, &tree_barrier_100);
    }
    for (int i = 0; i < 100; ++i) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end, NULL);
    elapsed = (end.tv_sec - start.tv_sec) * 1000000LL + end.tv_usec - start.tv_usec;
    printf("Time taken by tree barrier with 100 threads: %lld microseconds\n", elapsed);

    linear_barrier_destroy(&linear_barrier_100);
    tree_barrier_destroy(&tree_barrier_100);

    // Test with 1000 threads
    linear_barrier_t linear_barrier_1000;
    linear_barrier_init(&linear_barrier_1000, 1000);

    tree_barrier_t tree_barrier_1000;
    tree_barrier_init(&tree_barrier_1000, 1000);

    gettimeofday(&start, NULL);
    for (int i = 0; i < 1000; ++i) {
        pthread_create(&threads[i], NULL, linear_barrier_test, &linear_barrier_1000);
    }
    for (int i = 0; i < 1000; ++i) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end, NULL);
    elapsed = (end.tv_sec - start.tv_sec) * 1000000LL + end.tv_usec - start.tv_usec;
    printf("Time taken by linear barrier with 1000 threads: %lld microseconds\n", elapsed);

    gettimeofday(&start, NULL);
    for (int i = 0; i < 1000; ++i) {
        pthread_create(&threads[i], NULL, tree_barrier_test, &tree_barrier_1000);
    }
    for (int i = 0; i < 1000; ++i) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end, NULL);
    elapsed = (end.tv_sec - start.tv_sec) * 1000000LL + end.tv_usec - start.tv_usec;
    printf("Time taken by tree barrier with 1000 threads: %lld microseconds\n", elapsed);

    linear_barrier_destroy(&linear_barrier_1000);
    tree_barrier_destroy(&tree_barrier_1000);

    return 0;
}
