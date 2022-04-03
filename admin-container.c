#include<stdio.h>
#include<stdlib.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include<stdbool.h>
#include<pthread.h>
#include<assert.h>
#include<sys/mman.h>
#include<fcntl.h>

#define BUFFER 1000
#define SIZE 20

int cont = 0;

typedef struct __myarg_t{
	char petition[20];
	int port;
	int ecs_agent;
	char name[20];
} myarg_t;


int create_container(void *arg){
	myarg_t * args = arg;
	int create;
	struct sockaddr_in server;
	char petition[20], reply[50], port_str[5];
	FILE *fptr;
	char * token;
	char *contents = NULL;
    size_t len = 0;
	bool already_exist = false;

	create = socket(AF_INET, SOCK_STREAM, 0);
	if(create == -1){
		printf("Could not create petition socket\n");
	}
	puts("Petition socket created");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons( args->port );

	if( connect(create, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("Connect failed. Error");
		return 1;
	}
	puts("Connected");

	memset(petition, 0, 20);

	strcpy(petition, "create ");

	strcat(petition, args->name);

	printf("ENVIO: %s\n", petition);

	send(create, petition, strlen(petition), 0);

	memset(reply, 0, 50);

	recv(create, reply, 50, 0);

	printf("Respuesta: %s\n", reply);

	close(create);

	fptr = fopen("containers.txt", "r");

	while(getline(&contents, &len, fptr) != -1){
        //printf("%s\n", contents);
		token = strtok(contents, " ");
		if(strcmp(token, args->name) == 0){
			printf("si existe\n");
			already_exist = true;
			break;
		}
    }

	fclose(fptr);

	printf("SIGO VIVO\n");

	fptr = fopen("containers.txt", "a");

	if(!already_exist){
		printf("no existe\n");
		snprintf(port_str, 10, "%d", args->port);

		strcat(args->name, " ");
		strcat(args->name, port_str);
		strcat(args->name, "\n");

		fputs(args->name, fptr);
	}

	fclose(fptr);

	printf("SIGO MAS VIVO QUE MUERTO\n");

	free(args);
}

int send_petition(void *arg){
	myarg_t *args = arg;
	int send_socket, port;
	struct sockaddr_in server;
	char reply[50], buffer[BUFFER];
	FILE *fptr;
	FILE *fptr2;
	char *contents = NULL;
	char * token;
    size_t len = 0;
	bool delete = false;
	int del = 0, line = 0;

	fptr = fopen("containers.txt", "r");

    while (getline(&contents, &len, fptr) != -1){
        //printf("%s\n", contents);
		token = strtok(contents, " ");
		if(strcmp(token, args->name) == 0){
			token = strtok(NULL, " ");
			//printf("el token es: %s\n", token);
			break;
		}
		del++;
    }
	//printf("el tken sale: %s\n", token);
	port = atoi(token);

	//printf("puerto: %d para %s\n", port, petition);

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

	if(strcmp(args->petition, "delete") == 0){
		delete = true;
	}

	strcat(args->petition, " ");

	strcat(args->petition, args->name);

	send(send_socket, args->petition, strlen(args->petition), 0);

	memset(reply, 0, 50);

	recv(send_socket, reply, 50, 0);

	printf("Respuesta: %s\n", reply);

	if(delete){
		fptr = fopen("containers.txt", "r");
		fptr2 = fopen("temp.txt", "w");

		while((fgets(buffer, BUFFER, fptr)) != NULL){
			if(line != del){
				fputs(buffer, fptr2);
			}
			line++;
		}

		fclose(fptr2);
		fclose(fptr);

		remove("containers.txt");
		rename("temp.txt", "containers.txt");
	}

	close(send_socket);

	free(args);
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


int admin_container(void *arg){
	myarg_t * args = arg;
	char client_message[20], petition[20], name[20];
	bool received = false;
	int port, host = 0;
	char * token;
	puts("Connection accepted");

	//se separa
	
	int fd = shm_open("HOSTS", O_RDONLY, 0666);
	if(fd < 0){
		perror("shm_open()");
		return EXIT_FAILURE;
	}
	
	int *hosts = (int *)mmap(0, 100, PROT_READ, MAP_SHARED, fd, 0);
	printf("Receiver mapped address: %p\n", hosts);

	for(int i = 0; i < hosts[99]; i++){
		printf("host%d %d\n", i + 1, hosts[i]);
	}

	/*
	munmap(hosts, 100);

	close(fd);

	shm_unlink("HOSTS");
	*/

	received = false;

	while(!received) {
		memset( client_message, 0, 20 );
		//Receive a message from client
		if (recv(args->ecs_agent, client_message, 20, 0) > 0) {
			token = strtok(client_message, " ");
			strcpy(petition, token);
			if(strcmp(petition, "list") != 0){
				
				printf("OLA SOY CONT: %d\n", hosts[99]);
				token = strtok(NULL, " ");
				strcpy(name, token);
				printf("Petition: %s\nName: %s\n", petition, name);
				host = rand() % hosts[99];
				printf("Fue seleccionado el %d\n", host);
				port = hosts[host];
				printf("Name: host%d\nPort: %d\n", host + 1, port);

				if(strcmp(petition, "create") == 0){
					pthread_t create;
					myarg_t * args_c = malloc(sizeof(*args_c));
					args_c->port = port;
					strcpy(args_c->name, name);
					pthread_create( &create, NULL, (void*) create_container, args_c);
					//pthread_join(create, NULL);
					//create_container(host_name, port, name);
				} else if(strcmp(petition, "stop") == 0 || strcmp(petition, "delete") == 0){
					pthread_t send;
					myarg_t * args_s = malloc(sizeof(*args_s));
					strcpy(args_s->petition, petition);
					strcpy(args_s->name, name);
					pthread_create( &send, NULL, (void*) send_petition, args_s);
					//pthread_join(send, NULL);
					printf("EN PROCESO\n");
					//send_petition(petition, name);
				} 
				//Send the message back to client
				send(args->ecs_agent, client_message, strlen(client_message), 0);
				received = true;
			} else if(strcmp(petition, "list") == 0){
				pthread_t list;
				pthread_create( &list, NULL, (void*) list_containers, NULL);
				//pthread_join(list, NULL);
				//list_containers();
				received = true;
			}
			
		}
	}
}

int subscribe_host(void *arg){
	myarg_t * args = arg;
	char host_info[20];
	int port;
	char * token;

	puts("Connection accepted");

	memset(host_info, 0, 20);

	recv(args->ecs_agent, host_info, 20, 0);

	printf("Host: %s\n", host_info);

	int fd = shm_open("HOSTS", O_CREAT | O_RDWR, 0666);
	if (fd < 0){
		perror("shm_open()");
		return EXIT_FAILURE;
	}

	ftruncate(fd, 100);
	
	int *hosts = (int *)mmap(0, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	printf("Sender mapped address: %p\n", hosts);

	token = strtok(host_info, " ");
	token = strtok(NULL, " ");
	port = atoi(token);

	hosts[cont] = port;
	
	printf("cont es: %d\n", cont);
	//strcpy(hosts[cont], host_info);

	printf("shm sender: %d\n", hosts[cont]);

	cont++;
	hosts[99] = cont;

	munmap(hosts, 100);

	close(fd);

	printf("Finished subscribe host\n");

}

int main(int argc, char *argv[]) {
	int admin_socket, subs_socket, ecs_client, ecs_agent, c, read_size, cont, host = 0, port;
	char client_message[20], petition[20], name[20], host_info[20], buffer[20], host_name[5], port_char[5];
	char * token;
	bool recieved = false, separator = false;

	int readbytes;

	pid_t pid;

	pid = fork();

	if(pid == 0){
		struct sockaddr_in subs_host_server, ecs_agent_client;
		char message[1000], server_reply[2000];
		int i = 0;
		
		
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
		while(1){
			listen(subs_socket, 3);
			puts("Waiting for ecs-agents connections...");
			c = sizeof(struct sockaddr_in);

			ecs_agent = accept(subs_socket, (struct sockaddr *)&ecs_agent_client, (socklen_t*)&c);
			if(ecs_agent < 0) {
				perror("accept failed");
				return 1;
			}
			
			pthread_t host_connect;
			myarg_t arg_h;
			arg_h.ecs_agent = ecs_agent;
			pthread_create( &host_connect, NULL, (void*)subscribe_host, &arg_h);

		}
		

	} else{
		//close(pipeHost1[1]);
		//close(pipeHost2[1]);

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

			pthread_t agent_connect;
			myarg_t arg_cl;
			arg_cl.ecs_agent = ecs_client;
			pthread_create( &agent_connect, NULL, (void*)admin_container, &arg_cl);
			
		}
	}
	
	return 0;
}