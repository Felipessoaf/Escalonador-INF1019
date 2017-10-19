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
	struct HEAD *queue;
};

typedef struct node
{
	Process p;
	TAILQ_ENTRY(node) nodes;
} ProcessNode;

TAILQ_HEAD(HEAD, node) head1 = TAILQ_HEAD_INITIALIZER(head1);

void RemoveChildProcess()
{

}

void AddToQueue(struct HEAD *head, int stream[3], char *progName)
{
	int i;
	ProcessNode *new;
	new = (ProcessNode*) malloc(sizeof(ProcessNode));
	new->p.state = NEW;

	for(i = 0; i < 3; i++)
	{
		new->p.streams[i] = stream[i];
	}
	new->p.queue = head;

	strcpy(new->p.programName, progName);

	TAILQ_INSERT_HEAD(head, new, nodes);
}

int main()
{
	ProcessNode *current;
	ProcessNode *tmp;

	int childPid, result;
	int status;

	struct HEAD *head2 = (struct HEAD*)malloc(sizeof(struct HEAD));//TAILQ_HEAD_INITIALIZER(*head2);
	struct HEAD *head3 = (struct HEAD*)malloc(sizeof(struct HEAD));//TAILQ_HEAD_INITIALIZER(*head3);

	TAILQ_INIT(&head1);
	TAILQ_INIT(head2);
	TAILQ_INIT(head3);

	int stream[3];
	char programName[MAX_NAME];
	char *args[5];

	char pid[MAX_PID];
	sprintf(pid, "%ld", (long)getpid());
	args[0] = pid;
	args[4] = NULL;

	signal(SIGCHLD,RemoveChildProcess);

	printf("Digite o comando 'exec <nomedoprograma> (n1,n2,n3)'\n");
	result = scanf("exec %s (%d,%d,%d)", &programName, &stream[0], &stream[1], &stream[2]);
	while(result == 4)
	{
		//armazena os processos que o usuario quer rodar
		AddToQueue(&head1,stream, programName);

		result = scanf("exec %s (%d,%d,%d)", &programName, &stream[0], &stream[1], &stream[2]);
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
			sprintf(args[1], "%d", tmp->p.streams[0]);
			sprintf(args[2], "%d", tmp->p.streams[1]);
			sprintf(args[3], "%d", tmp->p.streams[2]);
			execv(tmp->p.programName, args);
			printf("erro no execv\n");
		}
	}

	//começa o escalonador
	while(head1.tqh_first || head2->tqh_first || head3->tqh_first)
	{
		printf("escalonando\n");

		kill(head1.tqh_first->p.pid, SIGCONT);
		TAILQ_REMOVE(&head1, head1.tqh_first, nodes);
	}

	while(1){}
	waitpid(-1, &status, 0);
	return 0;
}
