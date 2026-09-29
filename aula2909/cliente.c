/*****************************************
   cliente.c
*****************************************/

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

int sockfd_global;

/* thread que so fica recebendo mensagens do servidor e imprimindo */
void *recebeMensagens(void *arg) {
    int  sockfd = *(int *) arg;
    char recvline[MAXLINE];
    int  n;

    for (;;) {
        n = read(sockfd, recvline, MAXLINE - 1);
        if (n <= 0) {
            printf("\nConexao com o servidor encerrada.\n");
            exit(0);
        }
        recvline[n] = '\0';
        printf("\r%s\nDigite algo...\n", recvline);
        fflush(stdout);
    }
    return NULL;
}

void digitaAlgo(int sockfd) {
    char sendline[MAXLINE];
    int  n;
    printf("Digite algo...\n");
    while (fgets(sendline, MAXLINE, stdin) != NULL) {
        n = strlen(sendline);
        if (write(sockfd, sendline, n) != n) {
            printf("str_cli: writen error or socket\n");
            exit(3);
        }
    }
}

int main(int argc, char *argv[]) {
    int                 sockfd;
    struct sockaddr_in  serv_addr;
    struct hostent      *hp;

    if (argc < 2) {
        printf("Digite o nome do servidor!\n");
        exit(1);
    }

    printf("Trying to connect to server %s ...\n", argv[1]);
    hp = gethostbyname(argv[1]);
    if (hp == NULL) {
        printf("client: unknown host %s\n", argv[1]);
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    bcopy(hp->h_addr, (char *) &serv_addr.sin_addr, hp->h_length);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(SERV_TCP_PORT);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("client: can not open stream socket\n");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("client: can not connect to server\n");
        exit(2);
    }
    printf("\nOK!\n");

    sockfd_global = sockfd;

    pthread_t tid;
    pthread_create(&tid, NULL, recebeMensagens, &sockfd_global);

    digitaAlgo(sockfd);

    close(sockfd);
    return 0;
}
