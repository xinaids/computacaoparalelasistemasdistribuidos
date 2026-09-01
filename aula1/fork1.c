

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

/*
//exemplo 1 de fork
int main() {
	pid_t pid;
	pid = fork();
	if (pid == 0)
		printf("Eu sou um filho.\n");
	else
		printf("I am a Father.\n");
return 0;
}
*/

// exemplo de fork 2

int a=2;
int main(){
	pid_t pid;
	if ((pid = fork()) == 0){
		a = a + 2;
		printf("a no filho=%d\n", a);
	}
	else {
		sleep(2);
		a = a + 5;
		printf("a no papaizao=%d\n" , a);
	
	}
	return 0;
}
