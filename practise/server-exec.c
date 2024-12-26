#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

void error(char *msg) {
    perror(msg);
    exit(1);
}

void execute(char *buffer) {
    int i = 0;
    char *args[10];  // Correctly declare args as an array of string pointers
    pid_t pid;

    buffer[strcspn(buffer, "\n")] = '\0';  // Remove the newline character

    args[i] = strtok(buffer, " ");
    while (args[i] != NULL) {            
        args[++i] = strtok(NULL, " ");
    }
    args[i] = NULL;  // Ensure the array is NULL-terminated

    pid = fork();

    if (pid < 0) {
        // Fork failed
        error("fork failed");
    } else if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("exec failed");
        }
        exit(1); // Exit the child process if exec fails
    } else {
        // Parent process
        wait(NULL); // Wait for the child process to finish
    }
}

int main(int argc, char** argv) {
    int sockfd, new_sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    char buffer[255];

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Socket creation failed");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
        error("Binding error");

    listen(sockfd, 5);

    clilen = sizeof(cli_addr);
    new_sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (new_sockfd == -1) 
        error("Accepting Error");

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        if (read(new_sockfd, buffer, sizeof(buffer)) == -1)
            error("Reading Error");

        if (strncmp("!q", buffer, 2) == 0) {
            break;
        }

        execute(buffer);        
    }

    close(new_sockfd);
    close(sockfd);
    return 0;
}
