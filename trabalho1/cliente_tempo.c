/*
 * cliente_tempo.c - conecta no servidor, envia sua hora e
 * imprime a hora do servidor. Se o recurso estiver bloqueado,
 * o read() fica esperando ate o servidor atender.
 *
 * Uso: ./cliente_tempo [ip_servidor]   (padrao 127.0.0.1)
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#define SERV_HOST_ADDR "127.0.0.1"
#define SERV_PORT_ID 6090

struct tempo_rede {
    uint32_t sec;
    uint32_t usec;
};

/* imprime epoch como HH:MM:SS e DD/MM/AAAA (fuso do PC cliente) */
static void imprime_hora(const char *rotulo, time_t segundos)
{
    struct tm *tm = localtime(&segundos);
    char hora[16], data[16];

    strftime(hora, sizeof(hora), "%H:%M:%S", tm);
    strftime(data, sizeof(data), "%d/%m/%Y", tm);
    printf("%s\n%s\n%s\n", rotulo, hora, data);
}

int main(int argc, char *argv[])
{
    int sockid;
    struct sockaddr_in ssock_addr;
    struct timeval tp;
    struct tempo_rede t;
    const char *host = (argc > 1) ? argv[1] : SERV_HOST_ADDR;

    if ((sockid = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("erro criacao=%d\n", errno);
        exit(1);
    }

    bzero((char *)&ssock_addr, sizeof(ssock_addr));
    ssock_addr.sin_family = AF_INET;
    ssock_addr.sin_addr.s_addr = inet_addr(host);
    ssock_addr.sin_port = htons(SERV_PORT_ID);

    if (connect(sockid, (struct sockaddr *)&ssock_addr, sizeof(ssock_addr)) < 0) {
        printf("error connecting to server, error: %d\n", errno);
        exit(1);
    }

    gettimeofday(&tp, NULL);
    imprime_hora("client: local time is", tp.tv_sec);
    t.sec  = htonl((uint32_t)tp.tv_sec);
    t.usec = htonl((uint32_t)tp.tv_usec);
    write(sockid, &t, sizeof(t));

    /* bloqueia aqui ate o servidor liberar o recurso para este cliente */
    if (read(sockid, &t, sizeof(t)) != sizeof(t)) {
        printf("error reading new socket\n");
        exit(1);
    }
    imprime_hora("client: remote time is", (time_t)ntohl(t.sec));

    close(sockid);
    return 0;
}
