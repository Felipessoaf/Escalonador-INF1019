#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

typedef enum state
{
	new,
	ready,
	running,
	waiting,
	finished
} State;

typedef struct process
{
	int pid;
	State state;
	int streams[3];
} Process;

typedef struct node
{
	Process p;
	Process next;
	Process previous;
} ProcessNode;

typedef struct head
{
	Process first;
	Process current;
} ProcessHead;

int main()
{
	Process *f1, *f2, *f3;
	int i, stream[3], qtd1, qtd2, qtd3;

	f1 = (Process*)malloc(sizeof(Process));

//	do
//	{
//		for (i = 0; i < stream[0]; i++)
//		{
//			printf("%d\n", getpid());
//			sleep(1);
//		}
//	} while (1);

	return 0;
}
