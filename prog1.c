#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int i,j,k;
//	printf("vou parar\n");
	kill(getpid(), SIGSTOP);
//	printf("voltou\n");
	for (i = 0; i < strtol(argv[1], NULL, 10); i++)
	{
		printf("prog1-1: %d\n", getpid());
//		printf("vou dormir filho\n");
//		sleep(1);
//		if(i < strtol(argv[1], NULL, 10) - 1)
//		{
			kill(getpid(), SIGSTOP);
//		}
//		printf("filho acordou\n");
	}

	//avisa IO
//	kill(getppid(), SIGUSR1);

	for (j = 0; j < strtol(argv[2], NULL, 10); j++)
	{
		printf("prog1-2: %d\n", getpid());
//		sleep(1);
//		if(j < strtol(argv[1], NULL, 10) - 1)
//		{
			kill(getpid(), SIGSTOP);
//		}
	}

	//avisa IO
//	kill(getppid(), SIGUSR1);

	for (k = 0; k < strtol(argv[3], NULL, 10); k++)
	{
		printf("prog1-3: %d\n", getpid());
//		sleep(1);
//		if(k < strtol(argv[1], NULL, 10) - 1)
//		{
			kill(getpid(), SIGSTOP);
//		}
	}
	
//	kill(getppid(), SIGUSR2);
	return 0;
}

