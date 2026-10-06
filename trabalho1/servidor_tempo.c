/*
 * servidor_tempo.c - semaforo rudimentar
 *
 * O servidor atende UM cliente por vez. Depois de enviar a hora,
 * o recurso fica "bloqueado" por 2 segundos (sleep) antes de o
 * proximo cliente ser aceito. Os demais clientes ficam na fila
 * de conexoes pendentes (listen) e permanecem bloqueados no
 * read() ate chegar a vez deles.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

#define MY_PORT_ID 6090
#define BLOQUEIO_SEG 2

/* formato fixo na rede (timeval varia de tamanho entre plataformas) */
struct tempo_rede {
    uint32_t sec;
    uint32_t usec;
};

int main(void)
{
    int sockid, newsockid, i;
    struct sockaddr_in ssock_addr;
    struct timeval tp;
    struct tempo_rede t;

    if ((sockid = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("erro criacao= %d\n", errno);
        exit(1);
    }

    int opt = 1;
    setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero((char *)&ssock_addr, sizeof(ssock_addr));
    ssock_addr.sin_family = AF_INET;
    ssock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ssock_addr.sin_port = htons(MY_PORT_ID);

    if (bind(sockid, (struct sockaddr *)&ssock_addr, sizeof(ssock_addr)) < 0) {
        printf("error binding socket, error:%d\n", errno);
        exit(1);
    }

    /* backlog grande: clientes esperando a vez ficam aqui */
    if (listen(sockid, 50) < 0) {
        printf("erro listening: %d\n", errno);
        exit(1);
    }
    printf("Servidor aguardando conexoes na porta %d\n", MY_PORT_ID);

    for (i = 1; i <= 50000; i++) {
        newsockid = accept(sockid, (struct sockaddr *)0, (socklen_t *)0);
        if (newsockid < 0) {
            printf("error accepting socket, error: %d\n", errno);
            exit(1);
        }

        /* le tempo remoto */
        if (read(newsockid, &t, sizeof(t)) != sizeof(t)) {
            printf("error reading new socket\n");
            close(newsockid);
            continue;
        }
        printf("server: remote time is %u\n", ntohl(t.sec));

        /* hora local do servidor */
        gettimeofday(&tp, NULL);
        printf("server: local time is %ld (atendendo cliente %d)\n",
               (long)tp.tv_sec, i);
        t.sec  = htonl((uint32_t)tp.tv_sec);
        t.usec = htonl((uint32_t)tp.tv_usec);
        write(newsockid, &t, sizeof(t));
        close(newsockid);

        /* SEMAFORO: recurso bloqueado; proximo accept() so apos 2 s */
        printf("server: recurso bloqueado por %d s...\n", BLOQUEIO_SEG);
        sleep(BLOQUEIO_SEG);
        printf("server: recurso liberado\n");
    }
    close(sockid);
    return 0;
}
