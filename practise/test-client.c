/*
filename server_ipaddress portno

argv[0] filename
argv[1] server_ipaddress
argv[2] portno
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void handle_error(char* msg);

int main(int argc, char** argv) {

    int sockfd;
	struct sockaddr_in serv_addr;
	char buffer[255];
	int str_len;
	
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sockfd=socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
		handle_error("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
    
    if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
        handle_error("Connection failed");
    }

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        fgets(buffer, sizeof(buffer), stdin);
        if(write(sockfd, buffer,sizeof(buffer)) == -1) {
            handle_error("Error on writing!");
        }

        memset(buffer, 0, sizeof(buffer));

        if(read(sockfd, buffer, sizeof(buffer)) == -1) {
            handle_error("Error on reading!");
        }

        printf("Server: %s", buffer);

        if(strncmp("!q", buffer, 2) == 0) {
            break;
        }
    }
    
    close(sockfd);
    return 0;
}

void handle_error(char* msg) {
    perror(msg);
    exit(1);
}