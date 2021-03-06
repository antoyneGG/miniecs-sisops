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
	int admin;
} myarg_t;

int connection(void* arg){
	int admin = ((myarg_t *)arg)->admin;
	char admin_petition[20], reply[20], petition[20], name[20];
	char * token;
	bool received;

	puts("Connection accepted");
		
	received = false;

	while(!received){
		memset( admin_petition, 0, 20 );
		//Receive a message from client
		if (recv(admin, admin_petition, 20, 0) > 0) {
			printf("LLego: %s\n", admin_petition);
			
			token = strtok(admin_petition, " ");
			strcpy(petition, token);
			if(strcmp(petition, "list") != 0){
				token = strtok(NULL, " ");
				strcpy(name, token);
				printf("Petition: %s\nName: %s\n", petition, name);
			}
			received = true;
		}
	}
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
			strcpy(reply, "Container created");
		} else if(strcmp(petition, "stop") == 0){
			strcpy(reply, "Container stopped");
		} else if(strcmp(petition, "delete") == 0){
			strcpy(reply, "Container deleted");
		} else{
			strcpy(reply, "Listed");
		}
		wait(NULL);
		send(admin, reply, strlen(reply), 0);
		printf("Se envio la reply\n");
	}
}

int main(int argc, char *argv[]) {
	int agent, admin, c, read_size;
	struct sockaddr_in server, client;
	char info[20];

	agent = socket(AF_INET, SOCK_STREAM, 0);
	if (agent == -1) {
		printf("Could not create ecs-agent1");
	}
	puts("Ecs-agent1 created");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 6050 );

	if( connect(agent, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("Connect failed. Error");
		return 1;
	}
	puts("Connected");

    memset( info, 0, 20 );

    strcpy(info, "host3 5050");

    send(agent, info, strlen(info), 0);

	close(agent);

	agent = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 5050 );

	if (bind(agent, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("Ecs-agent bind failed. Error");
		return 1;
	}
	
	puts("Ecs-agent bind done\n");
	
	while(1){
		listen(agent, 10);
		puts("Waiting for ecs-agents connections...");
		c = sizeof(struct sockaddr_in);

		admin = accept(agent, (struct sockaddr *)&client, (socklen_t*)&c);
		if(admin < 0) {
			perror("accept failed");
			return 1;
		}
		
		pthread_t accepted_connection;
		myarg_t * arg_a = malloc(sizeof(myarg_t));
		arg_a->admin = admin;
		pthread_create( &accepted_connection, NULL, (void*) connection, arg_a);
	}

	return 0;
}