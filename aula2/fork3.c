#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    int y = 0;
    pid_t pid;

    if ((pid = fork()) < 0)
    {
        perror("fork");
        exit(1);
    }
    if (pid == 0)
    {
        //O código aqui dentro será executado no processo filho
        printf("pid do Filho: %d\n", getpid());
	for (int x = 0; x <= 100 ; x++){
	printf("Valor de x: %d\n", x);
	usleep(400000);
	}
    }
    else
    {
        //O código neste trecho será executado no processo pai
        printf("pid do Pai: %d\n", getpid());
	while (y < 50){
	printf("Valor de y: %d\n", y);
	y++;
	usleep(200000);
	}
    }

	
    printf("Esta regiao sera executada por ambos processos\n\n");
    //scanf("%d", &i);
    exit(0);
    return 0;
}
