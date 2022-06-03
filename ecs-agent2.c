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
	int admin = ((myarg_t *)arg)->admin, agent;
	char admin_petition[20], reply[20], petition[20], name[20], priority;
	struct sockaddr_in server;
	char * token;
	bool received;

	puts("Connection accepted");
		
	received = false;

	while(!received){
		memset( admin_petition, 0, 20 );
		//Receive a message from client
		if (recv(admin, admin_petition, 20, 0) > 0) {
			printf("LLego: %s\n", admin_petition);
			received = true;
		}
	}
	agent = socket(AF_INET, SOCK_STREAM, 0);
	if (agent == -1) {
		printf("Could not create ecs-agent re-sender");
	}
	puts("Ecs-agent2 re-sender created");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 9050 );

	if( connect(agent, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("Connect failed. Error");
		return 1;
	}
	puts("Connected");

	send(agent, admin_petition, strlen(admin_petition), 0);

	recv(agent, reply, 20, 0);

	close(agent);

	send(admin, reply, strlen(reply), 0);
	printf("Se envio la reply\n");
}

int main(int argc, char *argv[]) {
	int agent, admin, c, read_size;
	struct sockaddr_in server, client;
	char info[20];

	agent = socket(AF_INET, SOCK_STREAM, 0);
	if (agent == -1) {
		printf("Could not create ecs-agent1");
	}
	puts("Ecs-agent2 created");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 6050 );

	if( connect(agent, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("Connect failed. Error");
		return 1;
	}
	puts("Connected");

    memset( info, 0, 20 );

    strcpy(info, "host2 9090");

    send(agent, info, strlen(info), 0);

	close(agent);

	agent = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 9090 );

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