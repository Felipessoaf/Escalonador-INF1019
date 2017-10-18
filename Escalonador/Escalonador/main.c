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

void AddToQueue(struct HEAD *head, int newPid, int stream[3])
{
	int i;
	ProcessNode *new;
	new = (ProcessNode*) malloc(sizeof(ProcessNode));
	new->p.pid = newPid;
	new->p.state = NEW;

	for(i = 0; i < 3; i++)
	{
		new->p.streams[i] = stream[i];
	}
	new->p.queue = head;
	TAILQ_INSERT_HEAD(head, new, nodes);
}

int main()
{
	ProcessNode *current;

	struct HEAD *head2 = (struct HEAD*)malloc(sizeof(struct HEAD));//TAILQ_HEAD_INITIALIZER(*head2);
	struct HEAD *head3 = (struct HEAD*)malloc(sizeof(struct HEAD));//TAILQ_HEAD_INITIALIZER(*head3);

	TAILQ_INIT(&head1);
	TAILQ_INIT(head2);
	TAILQ_INIT(head3);

	int stream[3];
	char *args[4];

	char pid[20];
	sprintf(pid, "%ld", (long)getpid());
	args[0] = pid;

	signal(SIGCHLD,RemoveChildProcess);

	printf("Digite o comando 'exec <nomedoprograma> (n1,n2,n3)'\n");
	while(scanf("exec %s (%d,%d,%d)", &args[1], &stream[0], &stream[1], &stream[2]) == 4)
	{
		//armazena os processos que o usuario quer rodar
	}

	//começa o escalonador

	return 0;
}
