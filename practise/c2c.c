#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define PORT 3000
#define BUFFER_SIZE 2048

void handle_error(char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char** argv) {
    int serv_fd, cliA_fd, cliB_fd;
    struct sockaddr_in serv_addr, cliA_addr, cliB_addr;
    socklen_t clilen = sizeof(serv_addr);

    serv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(serv_fd < 0)
        handle_error("socket()");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(serv_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        handle_error("bind()");

    listen(serv_fd, 5);
    printf("Server listening on PORT %d...\n", PORT);

    cliA_fd = accept(serv_fd, (struct sockaddr *) &cliA_addr, &clilen);
    if(cliA_fd < 0)
        handle_error("client A error");
    else
        printf("Connect to client A (fd: %d)\n", cliA_fd);

    cliB_fd = accept(serv_fd, (struct sockaddr *) &cliB_addr, &clilen);
    if(cliB_fd < 0)
        handle_error("client B error");
    else
        printf("Connect to client B (fd: %d)\n", cliB_fd);

    while (1) {
        char buffer[BUFFER_SIZE] = {0};
        ssize_t bytes_received = recv(cliA_fd, buffer, sizeof(buffer) - 1, 0);
        if(bytes_received < 0) 
            handle_error("Receive failed!");
        buffer[bytes_received] = '\0';

        if(strncmp("exit", buffer, 4) == 0) {
            break;
        }

        send(cliB_fd, buffer, sizeof(buffer), 0);
    }
    

    close(cliA_fd);
    close(cliB_fd);
    close(serv_fd);

    return 0;
}