#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>

#define NUM_THREADS 1000
#include "treebarrier_pthreads.h"

typedef struct tree_barrier {
    pthread_barrier_t barrier;
    struct tree_barrier *parent;
    struct tree_barrier *left;
    struct tree_barrier *right;
} tree_barrier_t;

void tree_barrier_init(tree_barrier_t *barrier, int total) {
    pthread_barrier_init(&barrier->barrier, NULL, total);
    barrier->parent = NULL;
    barrier->left = NULL;
    barrier->right = NULL;
}

void tree_barrier_wait(tree_barrier_t *barrier) {
    if (barrier->parent != NULL) {
        pthread_barrier_wait(&barrier->parent->barrier);
    }

    pthread_barrier_wait(&barrier->barrier);
}

void tree_barrier_destroy(tree_barrier_t *barrier) {
    pthread_barrier_destroy(&barrier->barrier);
}

tree_barrier_t root;

void* tree_barrier_test(void* arg) {
    tree_barrier_wait(&root);
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    struct timeval start, end;

    tree_barrier_init(&root, NUM_THREADS);

    gettimeofday(&start, NULL);

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_create(&threads[i], NULL, tree_barrier_test, NULL);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&end, NULL);

    tree_barrier_destroy(&root);

    long long elapsed = (end.tv_sec - start.tv_sec) * 1000000LL + end.tv_usec - start.tv_usec;
    printf("Time taken by tree barrier: %lld microseconds\n", elapsed);

    return 0;
}
