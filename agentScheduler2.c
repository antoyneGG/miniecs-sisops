#include<stdio.h>
#include<stdlib.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include<stdbool.h>
#include<sys/wait.h>
#include<pthread.h>

typedef struct __myarg_t{
	int ecs_agent;
} myarg_t;

typedef struct execution{
    int priority;
    char name[20];
    char petition[20];
} execution;

execution scheduler[100];
int N = 0;

pthread_mutex_t schedulerLock = PTHREAD_MUTEX_INITIALIZER;

int priority_handler(){
    int i, head = 0;
    for(i = 0; i < N; i++){
        if(scheduler[i].priority < scheduler[head].priority){
            head = i;
        }
    }
    return head;
}

void scheduler_cleaner(int e){
    int i;
    for(i = e; i < N; i++){
        scheduler[i] = scheduler[i + 1];
    }
    N--;
}

void scheduler_handler(){
	int top;
	char reply[20], name[20], petition[20];
	while(1){
		while(N > 0){
			pthread_mutex_lock( &schedulerLock );
			top = priority_handler();
			strcpy(name, scheduler[top].name);
			strcpy(petition, scheduler[top].petition);
			scheduler_cleaner(top);
			pthread_mutex_unlock( &schedulerLock );

			pid_t pid = fork();
			if(pid == 0){
				if(strcmp(petition, "create") == 0){
					execlp("docker", "docker", "run", "-di", "--name", name, "ubuntu:latest", "/bin/bash", NULL);
				} else if(strcmp(petition, "stop") == 0){
					execlp("docker", "docker", "stop", name, NULL);
				} else if(strcmp(petition, "delete") == 0){
					execlp("docker", "docker", "rm", name, NULL);
				} else if(strcmp(petition, "list") == 0){
					execlp("docker", "docker", "ps", NULL);
				}
			} else{
				if(strcmp(petition, "create") == 0){
					strcpy(reply, "Container created: ");
				} else if(strcmp(petition, "stop") == 0){
					strcpy(reply, "Container stopped: ");
				} else if(strcmp(petition, "delete") == 0){
					strcpy(reply, "Container deleted: ");
				} else{
					strcpy(reply, "Listed");
				}
				printf("%s%s\n", reply, name);
			}
		}
		sleep(2);
	}
}

int connection(void* arg){
	int ecs_agent = ((myarg_t *)arg)->ecs_agent;
	char ecs_agent_petition[20], reply[20], petition[20], name[20];
	char * priority;
	char * token;
	bool received;

	puts("Connection accepted");
		
	received = false;

	while(!received){
		memset( ecs_agent_petition, 0, 20 );
		//Receive a message from client
		if (recv(ecs_agent, ecs_agent_petition, 20, 0) > 0) {
			printf("LLego: %s\n", ecs_agent_petition);
			
			token = strtok(ecs_agent_petition, " ");
			strcpy(petition, token);
			if(strcmp(petition, "list") != 0){
				token = strtok(NULL, " ");
				strcpy(name, token);
				token = strtok(NULL, " ");
				priority = token;
				printf("Petition: %s\nName: %s and Priority: %s\n", petition, name, priority);
			}
			received = true;
		}
	}

	pthread_mutex_lock( &schedulerLock );
	strcpy(scheduler[N].name, name);
    strcpy(scheduler[N].petition, petition);
    scheduler[N].priority = atoi(priority);
	N++;
	pthread_mutex_unlock( &schedulerLock );

	strcpy(reply, "Petition scheduled");
	send(ecs_agent, reply, strlen(reply), 0);
}

int main(int argc, char *argv[]) {
	int scheduler, ecs_agent, c, read_size;
	struct sockaddr_in server, client;
	char info[20];

	scheduler = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 9050 );

	if (bind(scheduler, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("AgentScheduler bind failed. Error");
		return 1;
	}
	
	puts("AgentScheduler bind done\n");

	pthread_t scheduler_thread;
	pthread_create( &scheduler_thread, NULL, (void*) scheduler_handler, NULL);
	
	while(1){
		listen(scheduler, 10);
		puts("Waiting for scheduler connections...");
		c = sizeof(struct sockaddr_in);

		ecs_agent = accept(scheduler, (struct sockaddr *)&client, (socklen_t*)&c);
		if(ecs_agent < 0) {
			perror("accept failed");
			return 1;
		}
		
		pthread_t accepted_connection;
		myarg_t * arg_a = malloc(sizeof(myarg_t));
		arg_a->ecs_agent = ecs_agent;
		pthread_create( &accepted_connection, NULL, (void*) connection, arg_a);
	}

	return 0;
}