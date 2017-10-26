#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

int cont;

void handler()
{
	cont = 1;
}

int main(int argc, char *argv[])
{
	int i,j,k;
	signal(SIGCONT, handler);
	cont = 0;
	for(;cont == 0;)
	{

	}
	cont = 0;
	printf("%d executando 1 rajada\n", getpid());
	for(;cont == 0;)
	{

	}
	cont = 0;

	//avisa IO
	printf("%d will enter IO\n", getpid());
	kill(getppid(), SIGUSR1);

	for(;cont == 0;)
	{

	}
	cont = 0;

	printf("%d executando 2 rajada\n", getpid());

	for(;cont == 0;)
	{

	}
	cont = 0;

	//avisa IO
	printf("%d will enter IO\n", getpid());
	kill(getppid(), SIGUSR1);

	for(;cont == 0;)
	{

	}
	cont = 0;

	printf("%d executando 3 rajada\n", getpid());

	for(;cont == 0;)
	{

	}
	cont = 0;

	return 0;
}

