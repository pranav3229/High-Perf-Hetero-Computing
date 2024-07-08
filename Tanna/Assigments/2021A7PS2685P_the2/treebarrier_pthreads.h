#ifndef TREE_BARRIER_PTHREADS_H
#define TREE_BARRIER_PTHREADS_H

#include <pthread.h>

typedef struct tree_barrier {
    pthread_barrier_t barrier;
    struct tree_barrier *parent;
    struct tree_barrier *left;
    struct tree_barrier *right;
} tree_barrier_t;

void tree_barrier_init(tree_barrier_t *barrier, int total);
void tree_barrier_wait(tree_barrier_t *barrier);
void tree_barrier_destroy(tree_barrier_t *barrier);


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

#endif /* TREE_BARRIER_PTHREADS_H */
