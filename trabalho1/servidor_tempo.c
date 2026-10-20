/*
 * servidor_tempo.c - semáforo rudimentar
 *
 * O servidor atende UM cliente por vez. Depois de enviar a hora,
 * o recurso fica "bloqueado" por 2 segundos (sleep) antes de o
 * próximo cliente ser aceito. Os demais clientes ficam na fila
 * de conexões pendentes (listen) e permanecem bloqueados no
 * read() até chegar a vez deles.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

#define MY_PORT_ID 6090
#define BLOQUEIO_SEG 2
#define FILA_ESPERA 50

/* cores ANSI */
#define RESET    "\033[0m"
#define NEGRITO  "\033[1m"
#define VERDE    "\033[32m"
#define AMARELO  "\033[33m"
#define AZUL     "\033[34m"
#define CIANO    "\033[36m"
#define VERMELHO "\033[31m"

/* formato fixo na rede (timeval varia de tamanho entre plataformas) */
struct tempo_rede {
    uint32_t sec;
    uint32_t usec;
};

/* prefixo com a hora atual: [HH:MM:SS] */
static void log_prefixo(void)
{
    time_t agora = time(NULL);
    struct tm *tm = localtime(&agora);
    char hora[16];

    strftime(hora, sizeof(hora), "%H:%M:%S", tm);
    printf(CIANO "[%s]" RESET " ", hora);
}

/* imprime epoch como HH:MM:SS - DD/MM/AAAA (mesmo formato do cliente) */
static void imprime_hora(const char *rotulo, const char *cor, time_t segundos)
{
    struct tm *tm = localtime(&segundos);
    char hora[16], data[16];

    strftime(hora, sizeof(hora), "%H:%M:%S", tm);
    strftime(data, sizeof(data), "%d/%m/%Y", tm);
    log_prefixo();
    printf(NEGRITO "%-20s" RESET " %s%s" RESET "  -  %s\n", rotulo, cor, hora, data);
}

static void linha(void)
{
    printf(AZUL "--------------------------------------------------" RESET "\n");
}

int main(void)
{
    int sockid, newsockid, i;
    struct sockaddr_in ssock_addr, cli_addr;
    socklen_t cli_len;
    struct timeval tp;
    struct tempo_rede t;

    if ((sockid = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf(VERMELHO "Erro ao criar o socket: %d" RESET "\n", errno);
        exit(1);
    }

    int opt = 1;
    setsockopt(sockid, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bzero((char *)&ssock_addr, sizeof(ssock_addr));
    ssock_addr.sin_family = AF_INET;
    ssock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ssock_addr.sin_port = htons(MY_PORT_ID);

    if (bind(sockid, (struct sockaddr *)&ssock_addr, sizeof(ssock_addr)) < 0) {
        printf(VERMELHO "Erro ao associar (bind) o socket: %d" RESET "\n", errno);
        exit(1);
    }

    /* backlog grande: clientes esperando a vez ficam aqui */
    if (listen(sockid, FILA_ESPERA) < 0) {
        printf(VERMELHO "Erro no listen: %d" RESET "\n", errno);
        exit(1);
    }

    printf("\n" NEGRITO AZUL "==================================================" RESET "\n");
    printf(NEGRITO "         SERVIDOR DE HORA - SEMÁFORO" RESET "\n");
    printf(NEGRITO AZUL "==================================================" RESET "\n");
    printf("  Porta:               " AMARELO "%d" RESET "\n", MY_PORT_ID);
    printf("  Bloqueio por cliente: " AMARELO "%d s" RESET "\n", BLOQUEIO_SEG);
    printf("  Fila de espera:      " AMARELO "%d conexões" RESET "\n", FILA_ESPERA);
    printf(NEGRITO AZUL "==================================================" RESET "\n\n");
    log_prefixo();
    printf(VERDE "Aguardando conexões..." RESET "\n\n");

    for (i = 1; i <= 50000; i++) {
        cli_len = sizeof(cli_addr);
        newsockid = accept(sockid, (struct sockaddr *)&cli_addr, &cli_len);
        if (newsockid < 0) {
            printf(VERMELHO "Erro ao aceitar a conexão: %d" RESET "\n", errno);
            exit(1);
        }

        linha();
        log_prefixo();
        printf(NEGRITO "Atendendo o cliente nº %d" RESET " (%s:%d)\n", i,
               inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

        /* lê a hora do cliente */
        if (read(newsockid, &t, sizeof(t)) != sizeof(t)) {
            log_prefixo();
            printf(VERMELHO "Erro ao ler os dados do cliente" RESET "\n");
            close(newsockid);
            continue;
        }
        imprime_hora("Hora do cliente:", AMARELO, (time_t)ntohl(t.sec));

        /* hora local do servidor */
        gettimeofday(&tp, NULL);
        imprime_hora("Hora do servidor:", VERDE, tp.tv_sec);

        t.sec  = htonl((uint32_t)tp.tv_sec);
        t.usec = htonl((uint32_t)tp.tv_usec);
        write(newsockid, &t, sizeof(t));
        close(newsockid);
        log_prefixo();
        printf("Resposta enviada ao cliente nº %d\n", i);

        /* SEMÁFORO: recurso bloqueado; próximo accept() só após 2 s */
        log_prefixo();
        printf(VERMELHO "Recurso BLOQUEADO por %d s..." RESET "\n", BLOQUEIO_SEG);
        sleep(BLOQUEIO_SEG);
        log_prefixo();
        printf(VERDE "Recurso LIBERADO" RESET "\n\n");
    }
    close(sockid);
    return 0;
}
