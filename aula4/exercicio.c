/*1 - 10^7 Produtor, Consumidor é o Vetor; usar semaforo para evitar condições corridas (algo que ja foi produzido)
funcao para avaliar se é primo ou não (buffer)
quando ele produzier 100^4 numeros, ele para. validar com o estudo de caso
rodar 10 vezes, pegar médiana de tempos, consumos, etc.
resultados, colocar em uma planilha do excel para avaliarmos com o professor
laço de repeticao com random, semwhait, sem_post, volta para o laço de repetição, verificar se tem espaço no  vetor para acessar. 

*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>


int *buffer;
int N, Np, Nc, M, contador;  // numero, numero produtor, numero consumidor, malloc | 
sem_t vazio, cheio;
pthread_mutex_t m0;


int ehPrimo(int num){
    if (num <= 1) return 0; // numero menores ou iguais a 1 não são primos
    for (int i = 2; i * i <= num; i++){
        if (num % i == 0){
            return 0; // encontra divisor, logo ñ é primo
        }
    }
    return 1; // éh primo
}

void *produtor(void *arg) {
    while(1){
    if (contador >= M) break;
    int numeroRandom = rand() % 10000000 + 1;
    sem_wait(&vazio);
    pthread_mutex_lock(&m0);
    for (int i = 0; i < N; i++) {
        if (buffer[i] == 0) {
        buffer[i] = numeroRandom;
        break;  // achou, para de procurar (break do for)
        }
    }
    pthread_mutex_unlock(&m0);
    sem_post(&cheio);
    }

    for (int i = 0; i < Nc; i++) {
        sem_post(&cheio);
    }

    return NULL;
}

void *consumidor(void *arg) {
    while (1) {
        if (contador >= M) break;

        sem_wait(&cheio);
        pthread_mutex_lock(&m0);

        int valor = 0;
        for (int i = 0; i < N; i++) {
            if (buffer[i] != 0) {
                valor = buffer[i];
                buffer[i] = 0;
                break;
            }
        }

        pthread_mutex_unlock(&m0);
        sem_post(&vazio);

        int primo = ehPrimo(valor);
        if (primo) {
            //printf("Numero primo: %d\n", valor);
        } else {
            //printf("Numero nao primo: %d\n", valor);
        }

        contador++;
    }
    
     for (int i = 0; i < Np; i++) {
        sem_post(&vazio);
    }  

    return NULL;
}

int main(int argc, char *argv[]) {
    
    if (argc != 4) {
        printf("Uso: %s N Np Nc\n", argv[0]);
        return 1;
    }

    N  = atoi(argv[1]);
    Np = atoi(argv[2]);
    Nc = atoi(argv[3]);
    M = 10000; // M = 10^4

    if (N <= 0 || Np <= 0 || Nc <= 0) {
        printf("N, Np e Nc devem ser maiores que zero\n");
        return 1;
    }

    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    buffer = calloc(N, sizeof(int));
    if (buffer == NULL) {
        printf("Erro ao alocar memoria para o buffer\n");
        return 1;
    }

    sem_init(&vazio, 0, N);
    sem_init(&cheio, 0, 0);
    pthread_mutex_init(&m0, NULL);

    srand(time(NULL));

    pthread_t *produtores = malloc(Np * sizeof(pthread_t));
    pthread_t *consumidores = malloc(Nc * sizeof(pthread_t));
    if (produtores == NULL || consumidores == NULL) {
        printf("Erro ao alocar memoria para as threads\n");
        return 1;
    }

    for (int i = 0; i < Np; i++) {
        pthread_create(&produtores[i], NULL, produtor, NULL);
    }
    for (int i = 0; i < Nc; i++) {
        pthread_create(&consumidores[i], NULL, consumidor, NULL);
    }

    for (int i = 0; i < Np; i++) {
        pthread_join(produtores[i], NULL);
    }
    for (int i = 0; i < Nc; i++) {
        pthread_join(consumidores[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &fim);
    double tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
    printf("Tempo: %f segundos\n", tempo);

    free(buffer);
    free(produtores);
    free(consumidores);
    sem_destroy(&vazio);
    sem_destroy(&cheio);
    pthread_mutex_destroy(&m0);

    return 0;
}