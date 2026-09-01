#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_t trd_1, trd_2;
pthread_mutex_t m0;

float shared = 0;

void *thread_0() {
    long int i;
    for (i = 0; i < 1000000; i++) {
        pthread_mutex_lock(&m0);
        shared = shared + 1;
        pthread_mutex_unlock(&m0);
    }
    return NULL;
}

void *thread_1() {
    long int i;
    for (i = 0; i < 2000000; i++) {
        pthread_mutex_lock(&m0);
        shared = shared + 1;
        pthread_mutex_unlock(&m0);
    }
    return NULL;
}

int main() {
    void *result;

    pthread_mutex_init(&m0, NULL);

    pthread_create(&trd_1, NULL, thread_0, NULL);
    pthread_create(&trd_2, NULL, thread_1, NULL);

    pthread_join(trd_1, &result);
    pthread_join(trd_2, &result);

    printf("Shared = %f\n", shared);

    pthread_mutex_destroy(&m0);

    return 0;
}
