#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define SERV_TCP_PORT  5555
#define MAXLINE        512
#define MAX_CLIENTS    32

typedef struct {
    int  sockfd;
    struct sockaddr_in addr;
} cliente_t;

cliente_t   clientes[MAX_CLIENTS];
int         n_clientes = 0;
pthread_mutex_t lock_clientes = PTHREAD_MUTEX_INITIALIZER;

void adiciona_cliente(int sockfd, struct sockaddr_in addr) {
    pthread_mutex_lock(&lock_clientes);
    if (n_clientes < MAX_CLIENTS) {
        clientes[n_clientes].sockfd = sockfd;
        clientes[n_clientes].addr   = addr;
        n_clientes++;
    }
    pthread_mutex_unlock(&lock_clientes);
}

void remove_cliente(int sockfd) {
    pthread_mutex_lock(&lock_clientes);
    for (int i = 0; i < n_clientes; i++) {
        if (clientes[i].sockfd == sockfd) {
            clientes[i] = clientes[n_clientes - 1];
            n_clientes--;
            break;
        }
    }
    pthread_mutex_unlock(&lock_clientes);
}

/* envia msg para todos os clientes conectados, exceto o remetente */
void broadcast(int sockfd_origem, char *msg, int n) {
    pthread_mutex_lock(&lock_clientes);
    for (int i = 0; i < n_clientes; i++) {
        if (clientes[i].sockfd != sockfd_origem) {
            write(clientes[i].sockfd, msg, n);
        }
    }
    pthread_mutex_unlock(&lock_clientes);
}

void *atende_cliente(void *arg) {
    cliente_t cli = *(cliente_t *) arg;
    free(arg);

    int  sockfd = cli.sockfd;
    char *ip    = inet_ntoa(cli.addr.sin_addr);
    char line[MAXLINE];
    char msg[MAXLINE + 64];
    int  n;

    printf("Cliente conectado --> IP: %s (socket %d)\n", ip, sockfd);

    for (;;) {
        n = read(sockfd, line, MAXLINE - 1);
        if (n <= 0) {
            printf("Cliente %s (socket %d) desconectou\n", ip, sockfd);
            remove_cliente(sockfd);
            close(sockfd);
            break;
        }
        line[n] = '\0';
        printf("Recebi de %s: %s", ip, line);

        int tam = snprintf(msg, sizeof(msg), "[%s]: %s", ip, line);
        broadcast(sockfd, msg, tam);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("server: can not open stream socket\n");
        exit(1);
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(SERV_TCP_PORT);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("server: can not bind local addr\n");
        exit(2);
    }

    listen(sockfd, MAX_CLIENTS);
    printf("Servidor aguardando conexoes na porta %d\n", SERV_TCP_PORT);

    for (;;) {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) {
            printf("server: accept error\n");
            continue;
        }

        cliente_t *arg = malloc(sizeof(cliente_t));
        arg->sockfd = newsockfd;
        arg->addr   = cli_addr;
        adiciona_cliente(newsockfd, cli_addr);

        pthread_t tid;
        pthread_create(&tid, NULL, atende_cliente, arg);
        pthread_detach(tid);
    }
}
