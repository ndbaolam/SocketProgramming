#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>

void handle_error(char* msg);

int main(int argc, char **argv) {
    
    if(argc < 2) {
        fprintf(stderr, "Port not provided. Program terminated!\n");
        exit(1);
    }

    int sockfd, new_sockfd, clilen;
    char buffer[255];

    struct sockaddr_in serv_addr, cli_addr;
    socklen_t client;    

    //create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("Create socket\n");

    if(sockfd == -1) {
        handle_error("Error opening socket");
    }

    bzero((char*) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    //Binding prot
    if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == - 1) {
        handle_error("Binding Failed!");
    } else {
        printf("Binding port\n");
    }

    if(listen(sockfd, 5) == -1) {
        handle_error("Cannot listen to port!");
    } else {
        printf("Server listening on port\n");
    }

    clilen = sizeof(cli_addr);
    new_sockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);

    if(new_sockfd == -1) {
        handle_error("Error on Accept");
    } else {
        printf("Server accepted connection from client\n");
    }

    while(1) {
        bzero(buffer, 255);
        if(read(new_sockfd, buffer, sizeof(buffer)) == -1) {
            handle_error("Error on Reading");
        }
        printf("Client: %s", buffer);

        bzero(buffer, 255);

        fgets(buffer, sizeof(buffer), stdin);
        if(write(new_sockfd, buffer, sizeof(buffer)) == -1) {
            handle_error("Error on Writing");
        }

        if(strncmp("!q", buffer, 2) == 0) {
            break;
        }
    }    

    close(sockfd);
    close(new_sockfd);
    bzero(buffer, sizeof(buffer));
    return 0;
}

void handle_error(char* msg) {
    perror(msg);
    exit(1);
}