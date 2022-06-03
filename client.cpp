#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <memory>
#include <vector>
#include <functional>
#include <algorithm>
#include <string.h>
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>

using namespace std;
pthread_mutex_t sendLock = PTHREAD_MUTEX_INITIALIZER;

int connection( int n ){
    //printf("EL Numero LLEGA ASI: %d\n", n);
    int sock;
	struct sockaddr_in server;
	char message[20] , server_reply[2000];
    char name[20], number[20];
    bool done = false;
	
    memset(name, 0, 20);
    memset(number, 0, 20);
    strcpy(name, "cont");
    snprintf(number, 20, "%d", n);
    strcat(name, number);
    cout << name << endl;

	//Create socket
	sock = socket(AF_INET , SOCK_STREAM, 0);
	if (sock == -1) {
		printf("Could not create socket");
	}
	puts("Socket created");
	
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons( 10000 );

	//Connect to remote server
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
		perror("connect failed. Error");
		return 1;
	}
	
	puts("Connected\n");
	
    memset(message, 0, 20);
	//keep communicating with server
    strcpy(message, "delete ");
    strcat(message, name);
    strcat(message, " ");
    strcat(message, number);
    
    //Send some data
    if( send(sock, message, strlen(message), 0) <= 0) {
        puts("Send failed");
        return 1;
    } else {
        printf("Send ok: %s\n", message);
    }
    

    //Receive a reply from the server
    memset( server_reply, 0, 2000 );
    //printf("me qeudo esperando el receive en %d\n", sock);
    if( recv(sock , server_reply , 2000 , 0) < 0) {
        puts("recv failed");
    } else {
        puts("recv ok");
    }
    //printf("ya recibi\n");

    puts("Server reply :");
    puts(server_reply);
    done = true;
    
    //printf(" enviando\n ");
    
    //printf(" ENVIADO en:%d \n", sock);
	
	close(sock);
    return 0;
}

int main(int argc , char *argv[]) {
    vector<thread> petitions;
    for(int i = 5; i < 10; i++){
        petitions.push_back(thread(connection, i));
    }
    for(int i = 1; i < 5; i++){
        petitions.push_back(thread(connection, i));
    }
    for_each(petitions.begin(), petitions.end(), mem_fn( &thread::join ));

	return 0;
}