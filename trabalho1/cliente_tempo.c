/*
 * cliente_tempo.c - conecta no servidor, envia sua hora e
 * imprime a hora do servidor. Se o recurso estiver bloqueado,
 * o read() fica esperando até o servidor atender.
 *
 * Uso: ./cliente_tempo [ip_servidor]   (padrão 127.0.0.1)
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

/* cores ANSI */
#define RESET    "\033[0m"
#define NEGRITO  "\033[1m"
#define VERDE    "\033[32m"
#define AMARELO  "\033[33m"
#define AZUL     "\033[34m"
#define VERMELHO "\033[31m"

struct tempo_rede {
    uint32_t sec;
    uint32_t usec;
};

/* imprime epoch como HH:MM:SS - DD/MM/AAAA (fuso do PC cliente) */
static void imprime_hora(const char *rotulo, const char *cor, time_t segundos)
{
    struct tm *tm = localtime(&segundos);
    char hora[16], data[16];

    strftime(hora, sizeof(hora), "%H:%M:%S", tm);
    strftime(data, sizeof(data), "%d/%m/%Y", tm);
    printf("  " NEGRITO "%-20s" RESET " %s%s" RESET "  -  %s\n", rotulo, cor, hora, data);
}

int main(int argc, char *argv[])
{
    int sockid;
    struct sockaddr_in ssock_addr;
    struct timeval tp;
    struct tempo_rede t;
    const char *host = (argc > 1) ? argv[1] : SERV_HOST_ADDR;

    printf("\n" NEGRITO AZUL "==================================================" RESET "\n");
    printf(NEGRITO "                CLIENTE DE HORA" RESET "\n");
    printf(NEGRITO AZUL "==================================================" RESET "\n");

    if ((sockid = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf(VERMELHO "Erro ao criar o socket: %d" RESET "\n", errno);
        exit(1);
    }

    bzero((char *)&ssock_addr, sizeof(ssock_addr));
    ssock_addr.sin_family = AF_INET;
    ssock_addr.sin_addr.s_addr = inet_addr(host);
    ssock_addr.sin_port = htons(SERV_PORT_ID);

    printf("  Conectando em %s:%d...\n", host, SERV_PORT_ID);
    if (connect(sockid, (struct sockaddr *)&ssock_addr, sizeof(ssock_addr)) < 0) {
        printf(VERMELHO "  Erro ao conectar ao servidor: %d" RESET "\n", errno);
        exit(1);
    }
    printf(VERDE "  Conectado!" RESET "\n\n");

    gettimeofday(&tp, NULL);
    imprime_hora("Hora do cliente:", AMARELO, tp.tv_sec);
    t.sec  = htonl((uint32_t)tp.tv_sec);
    t.usec = htonl((uint32_t)tp.tv_usec);
    write(sockid, &t, sizeof(t));

    /* bloqueia aqui até o servidor liberar o recurso para este cliente */
    printf("\n  Aguardando resposta do servidor...\n\n");
    if (read(sockid, &t, sizeof(t)) != sizeof(t)) {
        printf(VERMELHO "  Erro ao ler a resposta do servidor" RESET "\n");
        exit(1);
    }
    imprime_hora("Hora do servidor:", VERDE, (time_t)ntohl(t.sec));

    printf(NEGRITO AZUL "==================================================" RESET "\n\n");
    close(sockid);
    return 0;
}
