#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/queue.h>
#include <unistd.h>

#define MAX_NAME 30
#define MAX_PID 20
#define QUANTUM1 1
#define QUANTUM2 2
#define QUANTUM3 4

typedef enum state
{
	NEW,
	READY,
	RUNNING,
	WAITING,
	FINISHED
} State;

typedef struct process Process;
struct process
{
	int pid;
	char programName[MAX_NAME];
	State state;
	int streams[3];
	int timeInIO;
	struct HEAD *queue;
};

typedef struct node
{
	Process p;
	TAILQ_ENTRY(node) nodes;
} ProcessNode;

TAILQ_HEAD(HEAD, node) head1 = TAILQ_HEAD_INITIALIZER(head1);

ProcessNode *currentProcess;
struct HEAD *currenthead;

void RemoveChildProcess()
{
	//TODO: checar vazamento de memoria
	TAILQ_REMOVE(currenthead, currentProcess, nodes);
}

void ProcessEnteredIO()
{
	kill(currentProcess->p.pid, SIGSTOP);
	currentProcess->p.state = WAITING;
}

void AddToQueue(struct HEAD *head, int stream[3], char *progName)
{
	int i;
	ProcessNode *new;
	new = (ProcessNode*) malloc(sizeof(ProcessNode));
	new->p.state = NEW;
	new->p.timeInIO = 0;

	for(i = 0; i < 3; i++)
	{
		new->p.streams[i] = stream[i];
	}
	new->p.queue = head;

	strcpy(new->p.programName, progName);

	TAILQ_INSERT_HEAD(head, new, nodes);
}

void UpdateIO(struct HEAD *head)
{
	ProcessNode *tmp;
	TAILQ_FOREACH(tmp, head, nodes)
	{
		if(tmp->p.state == WAITING)
		{
			tmp->p.timeInIO += 1;
			if(tmp->p.timeInIO == 3)
			{
				tmp->p.state = READY;
				tmp->p.timeInIO = 0;
			}
		}
	}
}

int main()
{
	ProcessNode *tmp;

	int childPid;
	int status;

	int queue1CurrentQuantum, queue2CurrentQuantum, queue3CurrentQuantum;

	int stream[3];
	char programName[MAX_NAME];
	char *args[4];

	currenthead = &head1;
	struct HEAD *head2 = (struct HEAD*)malloc(sizeof(struct HEAD));//TAILQ_HEAD_INITIALIZER(*head2);
	struct HEAD *head3 = (struct HEAD*)malloc(sizeof(struct HEAD));//TAILQ_HEAD_INITIALIZER(*head3);

	TAILQ_INIT(&head1);
	TAILQ_INIT(head2);
	TAILQ_INIT(head3);

	args[3] = NULL;
	queue1CurrentQuantum = queue2CurrentQuantum = queue3CurrentQuantum = 0;

	signal(SIGCHLD,RemoveChildProcess);
	signal(SIGUSR1,ProcessEnteredIO);

	printf("Digite o comando 'exec <nomedoprograma> (n1,n2,n3)'\n");
	while(scanf(" exec %s (%d,%d,%d)", &programName, &stream[0], &stream[1], &stream[2]) == 4)
	{
		//armazena os processos que o usuario quer rodar
		AddToQueue(&head1,stream, programName);
	}

	//inicializa processos
	TAILQ_FOREACH(tmp, &head1, nodes)
	{
		childPid = fork();
		if(childPid != 0)
		{
			kill(childPid, SIGSTOP);
			tmp->p.pid = childPid;
		}
		else
		{
			printf("executando\n");
			sprintf(args[0], "%d", tmp->p.streams[0]);
			sprintf(args[1], "%d", tmp->p.streams[1]);
			sprintf(args[2], "%d", tmp->p.streams[2]);
			execv(tmp->p.programName, args);
			printf("erro no execv\n");
		}
	}

	//começa o escalonador
	while(1)
	{
		printf("escalonando\n");
		if(head1.tqh_first)
		{
			printf("fila 1\n");
			kill(head1.tqh_first->p.pid, SIGCONT);
			queue1CurrentQuantum += 1;
			if(queue1CurrentQuantum == QUANTUM1)
			{
				//Acabou o quantum, desce o processo pra fila 2
				queue1CurrentQuantum = 0;
			}
			//sleep(QUANTUM1);
			//UpdateIO(&head1);
		}
		else if(head2->tqh_first)
		{
			printf("fila 2\n");
			kill(head2->tqh_first->p.pid, SIGCONT);
			queue2CurrentQuantum += 1;
			if(queue2CurrentQuantum == QUANTUM2)
			{
				//Acabou o quantum, desce o processo pra fila 3
				queue2CurrentQuantum = 0;
			}
			//sleep(QUANTUM2);
//			UpdateIO(head2);
		}
		else if(head3->tqh_first)
		{
			printf("fila 3\n");
			kill(head3->tqh_first->p.pid, SIGCONT);
			queue3CurrentQuantum += 1;
			if(queue3CurrentQuantum == QUANTUM3)
			{
				//Acabou o quantum, processo permanece na fila
				queue3CurrentQuantum = 0;
			}
			//sleep(QUANTUM3);
//			UpdateIO(head3);
		}


		sleep(1);
		UpdateIO(&head1);
		UpdateIO(head2);
		UpdateIO(head3);
	}

	waitpid(-1, &status, 0);
	return 0;
}
