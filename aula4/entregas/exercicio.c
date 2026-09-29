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


int *buffer; //0 = posição livre, != 0 = ocupada
int N, Np, Nc, M, contador;  // numero, numero produtor, numero consumidor, malloc | 
sem_t vazio, cheio; // semaforos
pthread_mutex_t m0; // trava 


int ehPrimo(int num){
    if (num <= 1) return 0; // numero menores ou iguais a 1 não são primos
    for (int i = 2; i * i <= num; i++){ // testa se a raiz é quadrada
        if (num % i == 0){
            return 0; // encontra divisor, logo ñ é primo
        }
    }
    return 1; // éh primo
}

void *produtor(void *arg) {
    while(1){
        if (contador >= M) break;                        // (1) já terminou? sai
        int numeroRandom = rand() % 10000000 + 1;         // (2) sorteia entre 1 e 10^7
        sem_wait(&vazio);                                 // (3) espera ter espaço livre
        pthread_mutex_lock(&m0);                          // (4) trava o buffer só pra mim
        for (int i = 0; i < N; i++) {                      // (!) recebe dados. escrita. produtor
            if (buffer[i] == 0) {                         // (5) acha posição livre
                buffer[i] = numeroRandom;                 //     escreve o número
                break;
            }
        }
        pthread_mutex_unlock(&m0);                        // (6) libera o buffer
        sem_post(&cheio);                                 // (7) avisa: "+1 número disponível"
    }
    for (int i = 0; i < Nc; i++) {
        sem_post(&cheio);                                 // (8) "acorda" consumidores presos
    }
    return NULL;
}

void *consumidor(void *arg) {
    while (1) {
        if (contador >= M) break;
        sem_wait(&cheio);                       // espera ter algo pra consumir
        pthread_mutex_lock(&m0);
        int valor = 0;
        for (int i = 0; i < N; i++) {               // entrega os dados
            if (buffer[i] != 0) {                // acha posição OCUPADA
                valor = buffer[i];                // copia pra variável local
                buffer[i] = 0;                    // libera a posição
                break;
            }
        }
        pthread_mutex_unlock(&m0);
        sem_post(&vazio);                        // avisa: "+1 espaço livre"

        int primo = ehPrimo(valor);              // teste de primalidade
        if (primo) {
            //printf("Numero primo: %d\n", valor);      // comentado: silencia output
        } else {
            //printf("Numero nao primo: %d\n", valor);  // pro benchmark ficar limpo
        }
        contador++;                              // conta mais um processado
    }
    for (int i = 0; i < Np; i++) {
        sem_post(&vazio);                        // acorda produtores presos
    }
    return NULL;
}


int main(int argc, char *argv[]) {
    
    if (argc != 4) { // precisa 3 argumentos
        printf("Uso: %s N Np Nc\n", argv[0]);
        return 1;
    }

    N  = atoi(argv[1]);
    Np = atoi(argv[2]);
    Nc = atoi(argv[3]);
    M = 10000; // M = 10^4

    if (N <= 0 || Np <= 0 || Nc <= 0) { // valores positivos
        printf("N, Np e Nc devem ser maiores que zero\n");
        return 1;
    }

    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio); // relogio de inicio

    buffer = calloc(N, sizeof(int)); // cria e zera o buffer compartilhado (vetor)
    if (buffer == NULL) {
        printf("Erro ao alocar memoria para o buffer\n");
        return 1;
    }

    sem_init(&vazio, 0, N); // inicializacao semaforos
    sem_init(&cheio, 0, 0);
    pthread_mutex_init(&m0, NULL);

    srand(time(NULL));

    // aloca vetores de threads (tamanho variavel, definido em tempo de execucao)

    pthread_t *produtores = malloc(Np * sizeof(pthread_t)); // cria as threads , produtor
    pthread_t *consumidores = malloc(Nc * sizeof(pthread_t)); // consumidor
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
    } // espera todas terminarem
    
    clock_gettime(CLOCK_MONOTONIC, &fim); // fim do relogio
    double tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
    printf("Tempo: %f segundos\n", tempo);

    free(buffer); // destruir vetor...  
    free(produtores);
    free(consumidores); // consumidor...
    sem_destroy(&vazio);
    sem_destroy(&cheio);
    pthread_mutex_destroy(&m0);

    return 0;
}