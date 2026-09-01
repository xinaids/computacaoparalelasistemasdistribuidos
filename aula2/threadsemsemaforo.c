#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_t trd_1, trd_2;

float shared = 0;

void *thread_0() {
	long int i;
	for(i=0; i<1000000; i++)
		shared = shared + 1;
}

void *thread_1() {
	long int i;
	for (i=0; i<2000000; i++)
		shared = shared + 1;
}

int main() {
	int result;
	result = pthread_create(&trd_1, NULL, thread_0, NULL);
	result = pthread_create(&trd_2, NULL, thread_1, NULL);
	pthread_join(trd_1, (void **) &result);
	pthread_join(trd_2, (void **) &result);
	printf("Shared = %f\n", shared);
}



