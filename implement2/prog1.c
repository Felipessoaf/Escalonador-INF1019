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
cont =1;	
}

int main(int argc, char *argv[])
{
	int i,j,k;
cont = 0;
//	printf("vou parar\n");
	//kill(getpid(), SIGSTOP);
//	printf("voltou\n");
	signal(SIGCONT, handler);
	printf("%d executando\n", getpid());
	for (;;)
	{
		if(cont)
{
break;
}
		//while(1)
//{}
//		printf("vou dormir filho\n");
//		sleep(1);
//		if(i < strtol(argv[1], NULL, 10) - 1)
//		{
//			kill(getpid(), SIGSTOP);
//		}
//		printf("filho acordou\n");
	}

	//avisa IO
//	printf("will enter IO\n");
	kill(getppid(), SIGUSR1);
//	kill(getpid(), SIGSTOP);

	printf("%d executando\n", getpid());
	for (;;)
	{
if(cont)
{
break;
}
		//printf("prog1-2: %d\n", getpid());
//		sleep(1);
//		if(j < strtol(argv[2], NULL, 10) - 1)
//		{
			//kill(getpid(), SIGSTOP);
//		}
	}

	//avisa IO
//	printf("will enter IO\n");
	kill(getppid(), SIGUSR1);
	//kill(getpid(), SIGSTOP);

	printf("%d executando\n", getpid());
	for (;;)//k = 0; k < strtol(argv[3], NULL, 10); k++)
	{
if(cont)
{
break;
}
		//printf("prog1-3: %d\n", getpid());
//		sleep(1);
//		if(k < strtol(argv[3], NULL, 10) - 1)
//		{
			//kill(getpid(), SIGSTOP);
//		}
	}
	
//	kill(getppid(), SIGUSR2);
	return 0;
}

