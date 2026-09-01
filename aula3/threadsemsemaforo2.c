#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_t trd_1, trd_2;
float shared = 0;

void *thread_0() {
    long int i;
    pid_t meu_pid = getpid();
    for(i=0; i<1000000; i++)
        shared = shared + 1;
    printf("O PID deste processo e: %d\n", meu_pid);
    return NULL;
}

void *thread_1() {
    long int i;
    pid_t meu_pid = getpid();
    for (i=0; i<2000000; i++)
        shared = shared + 1;
    printf("O PID deste processo e: %d\n", meu_pid);
    return NULL;
}

int main() {
    void *result;  
    pthread_create(&trd_1, NULL, thread_0, NULL);
    pthread_create(&trd_2, NULL, thread_1, NULL);
    pthread_join(trd_1, &result);   
    pthread_join(trd_2, &result);
    printf("Shared = %f\n", shared);
    return 0;
}
