#include<stdio.h>
#include<stdlib.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include<stdbool.h>

#define SIZE 20

int create_container(char host[5], int port, char name[20]){
	int create;
	struct sockaddr_in server;
	char petition[20], reply[50], port_str[5];
	FILE *fptr;

	create = socket(AF_INET, SOCK_STREAM, 0);
	if(create == -1){
		printf("Could not create petition socket\n");
	}
	puts("Petition socket created");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons( port );

	if( connect(create, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("Connect failed. Error");
		return 1;
	}
	puts("Connected");

	memset(petition, 0, 20);

	strcpy(petition, "create ");

	strcat(petition, name);

	send(create, petition, strlen(petition), 0);

	memset(reply, 0, 50);

	recv(create, reply, 50, 0);

	printf("Respuesta: %s\n", reply);

	close(create);

	fptr = fopen("containers.txt", "a");

	snprintf(port_str, 10, "%d", port);

	strcat(name, " ");
	strcat(name, port_str);
	strcat(name, "\n");

	fputs(name, fptr);

	fclose(fptr);
}

int send_petition(char petition[20], char name[20]){
	int send_socket, port;
	struct sockaddr_in server;
	char reply[50];
	FILE *fptr;
	char *contents = NULL;
	char * token;
    size_t len = 0;

	fptr = fopen("containers.txt", "r");

    while (getline(&contents, &len, fptr) != -1){
        //printf("%s\n", contents);
		token = strtok(contents, " ");
		if(strcmp(token, name) == 0){
			token = strtok(NULL, " ");
			//printf("el token es: %s\n", token);
		}
    }
	port = atoi(token);

	//printf("puerto: %d\n", port);

	fclose(fptr);

	send_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(send_socket == -1){
		printf("Could not create petition socket\n");
	}
	puts("Petition socket created");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons( port );

	if( connect(send_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("Connect failed. Error");
		return 1;
	}
	puts("Connected");

	strcat(petition, " ");

	strcat(petition, name);

	send(send_socket, petition, strlen(petition), 0);

	memset(reply, 0, 50);

	recv(send_socket, reply, 50, 0);

	printf("Respuesta: %s\n", reply);

	close(send_socket);
}

int list_containers(){
	int list;
	struct sockaddr_in server;
	char petition[20], reply[50];

	list = socket(AF_INET, SOCK_STREAM, 0);
	if(list == -1){
		printf("Could not create petition socket\n");
	}
	puts("Petition socket created");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons( 8080 );

	if( connect(list, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("Connect failed. Error");
		return 1;
	}
	puts("Connected");

	memset(petition, 0, 20);

	strcpy(petition, "list");

	send(list, petition, strlen(petition), 0);

	memset(reply, 0, 50);

	recv(list, reply, 50, 0);

	printf("Respuesta: %s\n", reply);

	close(list);
}

int main(int argc, char *argv[]) {
	int admin_socket, subs_socket, ecs_client, ecs_agent, c, read_size, cont, pipeHost1[2], pipeHost2[2], host = 0, port;
	char client_message[20], petition[20], name[20], host_info[20], buffer[20], host_name[5], port_char[5];
	char * token;
	bool recieved = false, separator = false;

	pipe(pipeHost1);
	pipe(pipeHost2);

	int readbytes;

	pid_t pid;

	pid = fork();

	if(pid == 0){
		struct sockaddr_in subs_host_server, ecs_agent_client;
		char message[1000], server_reply[2000];

		close(pipeHost1[0]);
		close(pipeHost2[0]);
		
		//Create socket
		subs_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (subs_socket == -1) {
			printf("Could not create subscribe_host");
		}
		puts("Subscribe_host created");
		
		subs_host_server.sin_addr.s_addr = INADDR_ANY;
		subs_host_server.sin_family = AF_INET;
		subs_host_server.sin_port = htons( 6050 );  //no deberia ser
		

		//Connect to remote subs_host_server
		if (bind(subs_socket, (struct sockaddr *)&subs_host_server, sizeof(subs_host_server)) < 0) {
			perror("Subs host bind failed. Error");
			return 1;
		}
		
		puts("Subs host bind done\n");

		listen(subs_socket, 3);
		puts("Waiting for ecs-agents connections...");
		c = sizeof(struct sockaddr_in);

		ecs_agent = accept(subs_socket, (struct sockaddr *)&ecs_agent_client, (socklen_t*)&c);
		if(ecs_agent < 0) {
			perror("accept failed");
			return 1;
		}
		puts("Connection accepted");

		memset(host_info, 0, 20);

		recv(ecs_agent, host_info, 20, 0);

		printf("Host: %s\n", host_info);
		write(pipeHost1[1], host_info, SIZE);
		close(pipeHost1[1]);
		

	} else{
		close(pipeHost1[1]);
		char host1_info[20], host2_info[20];
		struct sockaddr_in server, client;

		//Create socket
		// AF_INET (IPv4 protocol), AF_INET6 (IPv6 protocol) 
		// SOCK_STREAM: TCP(reliable, connection oriented)
		// SOCK_DGRAM: UDP(unreliable, connectionless)
		// Protocol value for Internet Protocol(IP), which is 0
		admin_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (admin_socket == -1) {
			printf("Could not create socket");
		}
		puts("Socket created");
		
		//Prepare the sockaddr_in structure
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_port = htons( 10000 );
		
		//Bind the socket to the address and port number specified
		if( bind(admin_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
			//print the error message
			perror("bind failed. Error");
			return 1;
		}
		puts("bind done");
		
		read(pipeHost1[0], host1_info, SIZE);

		while(1){
			//Listen
			listen(admin_socket, 3);
			
			//Accept and incoming connection
			puts("Waiting for incoming connections...");
			c = sizeof(struct sockaddr_in);
			
			//accept connection from an incoming client
			ecs_client = accept(admin_socket, (struct sockaddr *)&client, (socklen_t*)&c);
			if (ecs_client < 0) {
				perror("accept failed");
				return 1;
			}
			puts("Connection accepted");

			recieved = false;

			while(!recieved) {
				memset( client_message, 0, 20 );
				//Receive a message from client
				if (recv(ecs_client, client_message, 20, 0) > 0) {
					token = strtok(client_message, " ");
					strcpy(petition, token);
					if(strcmp(petition, "list") != 0){
						token = strtok(NULL, " ");
						strcpy(name, token);
						printf("Petition: %s\nName: %s\n", petition, name);
						host = rand() % 2;
						printf("Fue seleccionado el %d\n", host);
						memset(host_info, 0, SIZE);
						memset(host_name, 0, 5);
						if(host == 0 || host == 1){
							strcpy(host_info, host1_info);
							printf("Host: %s\n", host1_info);							
						} else if(host == 1){
							recieved = true;
							continue;
							strcpy(host_info, host2_info);
							printf("Host: %s\n", host_info);
						}
						token = strtok(host_info, " ");
						strcpy(host_name, token);
						token = strtok(NULL, " ");
						port = atoi(token);
						printf("Name: %s\nPort: %d\n", host_name, port);
						if(strcmp(petition, "create") == 0){
							create_container(host_name, port, name);
						} else if(strcmp(petition, "stop") == 0 || strcmp(petition, "delete") == 0){
							send_petition(petition, name);
						} 
						//Send the message back to client
						send(ecs_client, client_message, strlen(client_message), 0);
						recieved = true;
					} else if(strcmp(petition, "list") == 0){
						list_containers();
					}
					
				}
			}
			
		}
	}
	
	return 0;
}