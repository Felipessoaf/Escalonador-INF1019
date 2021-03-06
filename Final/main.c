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
#include <errno.h>

#define MAX_NAME 30
#define MAX_PID 20
#define QUANTUM1 1
#define QUANTUM2 2
#define QUANTUM3 4
//#define ECLIPSE

typedef enum state
{
	NEW,
	READY,
	RUNNING,
	WAITING,
	FINISHED
} State;

typedef enum schedulerState
{
	NONE,
	QUEUE1,
	QUEUE2,
	QUEUE3
} SchedulerState;

typedef struct process Process;
struct process
{
	int pid;
	char programName[MAX_NAME];
	State state;
	int streams[3];
	int timeInIO;
	int justChangedQueue;
	int wasInIO;
	int currentStream;
	struct HEAD *queue;
};

typedef struct node
{
	Process p;
	TAILQ_ENTRY(node) nodes;
} ProcessNode;

TAILQ_HEAD(HEAD, node) head1 = TAILQ_HEAD_INITIALIZER(head1);
struct HEAD *head2;
struct HEAD *head3;

ProcessNode *currentProcess;
struct HEAD *currenthead;
SchedulerState schedulerState;

//exec prog1 (1,2,3)

void DeleteChildProcess()
{
	if(currentProcess != NULL)
	{
		TAILQ_REMOVE(currenthead, currentProcess, nodes);

		free(currentProcess);
		currentProcess = NULL;
		schedulerState = NONE;
	}
}

void ProcessEnteredIO()
{
	if(currentProcess)
	{
		currentProcess->p.state = WAITING;
		currentProcess->p.timeInIO = 0;
		if(!currentProcess->p.justChangedQueue)
		{
			if(schedulerState == QUEUE2)
			{
				currentProcess->p.queue = &head1;

				TAILQ_REMOVE(currenthead, currentProcess, nodes);
				TAILQ_INSERT_TAIL(&head1, currentProcess, nodes);
			}
			else if(schedulerState == QUEUE3)
			{
				currentProcess->p.queue = head2;

				TAILQ_REMOVE(currenthead, currentProcess, nodes);
				TAILQ_INSERT_TAIL(head2, currentProcess, nodes);
			}
		}
	}
	schedulerState = NONE;
	currentProcess = NULL;
}

void AddToQueue(struct HEAD *head, int stream[3], char *progName)
{
	int i;
	ProcessNode *new;
	new = (ProcessNode*) malloc(sizeof(ProcessNode));
	new->p.state = NEW;
	new->p.timeInIO = 0;
	new->p.justChangedQueue = 0;
	new->p.wasInIO = 0;
	new->p.currentStream = 0;

	for(i = 0; i < 3; i++)
	{
		new->p.streams[i] = stream[i];
	}
	new->p.queue = head;

	strcpy(new->p.programName, progName);

	TAILQ_INSERT_TAIL(head, new, nodes);
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
				tmp->p.wasInIO = 1;
			}
		}
	}
}

int CheckReadyNew(struct HEAD *head)
{
	ProcessNode *tmp;
	TAILQ_FOREACH(tmp, head, nodes)
	{
		if(tmp->p.state == NEW || tmp->p.state == READY)
		{
			return 1;
		}
	}

	return 0;
}

int CheckWaiting(struct HEAD *head)
{
	ProcessNode *tmp;
	TAILQ_FOREACH(tmp, head, nodes)
	{
		if(tmp->p.state == WAITING)
		{
			return 1;
		}
	}

	return 0;
}

int CheckUpdatingIO()
{
	return !CheckReadyNew(&head1) && !CheckReadyNew(head2) && !CheckReadyNew(head3) && (CheckWaiting(&head1) || CheckWaiting(head2) || CheckWaiting(head3));
}

ProcessNode * GetReadyNew(struct HEAD *head)
{
	ProcessNode *tmp;
	TAILQ_FOREACH(tmp, head, nodes)
	{
		if(tmp->p.state == NEW || tmp->p.state == READY)
		{
			return tmp;
		}
	}

	return NULL;
}

void interpreter()
{
	int childPid;

	ProcessNode *tmp;

	int stream[3];

	char programName[MAX_NAME];
	char arg1[20];
	char arg2[20];
	char arg3[20];

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
			tmp->p.pid = childPid;
		}
		else
		{
			sprintf(arg1, "%d", tmp->p.streams[0]);
			sprintf(arg2, "%d", tmp->p.streams[1]);
			sprintf(arg3, "%d", tmp->p.streams[2]);
#ifdef ECLIPSE
			if(execl("/home/felipessoaf/Desktop/EscalonadorRepo/implement2/prog1", tmp->p.programName, arg1, arg2, arg3, (char*)NULL) == -1)
#else
				if(execl(tmp->p.programName, tmp->p.programName, arg1, arg2, arg3, (char*)NULL) == -1)
#endif
				{
					printf("erro: exec\n");
				}
		}
	}
}

int schedulerAux(int *currentQuantum, int *shouldSleep, int quantumMax, int queue)
{
	int newQueuePrint = 0;

	if(schedulerState == NONE)
	{
		*currentQuantum = 0;
	}
	if(*currentQuantum == quantumMax)
	{
		if(currentProcess)
		{
			if(queue == 1)
			{
				//Acabou o quantum, desce o processo pra fila 2
				TAILQ_REMOVE(currenthead, currentProcess, nodes);
				TAILQ_INSERT_TAIL(head2, currentProcess, nodes);
				currentProcess->p.queue = head2;
				currentProcess->p.justChangedQueue = 1;
			}
			else if(queue == 2)
			{
				//Acabou o quantum, desce o processo pra fila 3
				TAILQ_REMOVE(currenthead, currentProcess, nodes);
				TAILQ_INSERT_TAIL(head3, currentProcess, nodes);
				currentProcess->p.queue = head3;
				currentProcess->p.justChangedQueue = 1;
			}
			currentProcess->p.state = READY;
		}
		currentProcess = NULL;

		*currentQuantum = 0;
		schedulerState = NONE;

		*shouldSleep = 0;
	}
	else
	{
		if(queue == 1)
		{
			schedulerState = QUEUE1;
			currenthead = &head1;
		}
		else if(queue == 2)
		{
			schedulerState = QUEUE2;
			currenthead = head2;
		}
		else if(queue == 3)
		{
			schedulerState = QUEUE3;
			currenthead = head3;
		}

		if(currentProcess == NULL)
		{
			currentProcess = GetReadyNew(currenthead);
			newQueuePrint = 1;
		}
		else
		{
			currentProcess->p.justChangedQueue = 0;
		}

		if(currentProcess)
		{
			if(currentProcess->p.streams[currentProcess->p.currentStream] == 0)
			{
				currentProcess->p.currentStream += 1;
				kill(currentProcess->p.pid, SIGCONT);
				sleep(1);
				*shouldSleep = 0;
				return 0;
			}
		}
		else
		{
			return 0;
		}

		if(currentProcess && newQueuePrint)
		{
			if(currentProcess->p.state == NEW || currentProcess->p.wasInIO)
			{
				currentProcess->p.wasInIO = 0;
				printf("\nFila %d\nProcesso: %s | Rajada: %d | Tempo restante: %d\n", queue,currentProcess->p.programName,
						currentProcess->p.currentStream + 1, currentProcess->p.streams[currentProcess->p.currentStream]);
				kill(currentProcess->p.pid, SIGCONT);
			}
			else
			{
				printf("\nFila %d\nProcesso: %s | Rajada: %d | Tempo restante: %d | PID: %d\n", queue,currentProcess->p.programName,
						currentProcess->p.currentStream + 1, currentProcess->p.streams[currentProcess->p.currentStream], currentProcess->p.pid);
			}
			newQueuePrint = 0;
			currentProcess->p.state = RUNNING;
		}

		*currentQuantum += 1;
		if(currentProcess)
		{
			currentProcess->p.streams[currentProcess->p.currentStream] -= 1;
		}

		*shouldSleep = 1;
	}
	return 1;
}

void scheduler()
{
	int shouldSleep;
	int newQueuePrint = 0;

	int queue1CurrentQuantum, queue2CurrentQuantum, queue3CurrentQuantum;

	queue1CurrentQuantum = queue2CurrentQuantum = queue3CurrentQuantum = 0;

	while(1)
	{
		if((CheckReadyNew(&head1) && schedulerState == NONE) || schedulerState == QUEUE1)
		{
			if(!schedulerAux(&queue1CurrentQuantum, &shouldSleep, QUANTUM1, 1))
			{
				continue;
			}
		}
		else if((CheckReadyNew(head2) && schedulerState == NONE) || schedulerState == QUEUE2)
		{
			if(!schedulerAux(&queue2CurrentQuantum, &shouldSleep, QUANTUM2, 2))
			{
				continue;
			}
		}
		else if((CheckReadyNew(head3) && schedulerState == NONE) || schedulerState == QUEUE3)
		{
			if(!schedulerAux(&queue3CurrentQuantum, &shouldSleep, QUANTUM3, 3))
			{
				continue;
			}
		}

		if(CheckUpdatingIO())
		{
			shouldSleep = 1;
		}
		if(shouldSleep)
		{
			sleep(1);
			UpdateIO(&head1);
			UpdateIO(head2);
			UpdateIO(head3);
		}

		if(head1.tqh_first == NULL && head2->tqh_first == NULL && head3->tqh_first == NULL)
		{
			printf("\nNenhum processo sobrando\n");
			return;
		}
	}
}

int main()
{
	currenthead = &head1;
	head2 = (struct HEAD*)malloc(sizeof(struct HEAD));
	head3 = (struct HEAD*)malloc(sizeof(struct HEAD));

	TAILQ_INIT(&head1);
	TAILQ_INIT(head2);
	TAILQ_INIT(head3);

	schedulerState = NONE;

	struct sigaction sa;
	sa.sa_handler = &DeleteChildProcess;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &sa, 0) == -1) {
	  perror(0);
	  exit(1);
	}

	signal(SIGUSR1,ProcessEnteredIO);

	//chama o interpretador
	interpreter();

	sleep(1);

	//começa o escalonador
	scheduler();

	return 0;
}
