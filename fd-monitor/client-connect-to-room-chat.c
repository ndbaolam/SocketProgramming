#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd, activity;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE], name[128];
    fd_set readfds;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Enter username: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';    

    if(send(sockfd, name, strlen(name), 0) < 0) {
        perror("send()");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (1) {        
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sockfd, &readfds); 

        activity = select(sockfd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0 && errno != EINTR) {
            perror("select() failed");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = '\0';

            if (strlen(buffer) > 0) {
                if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
                    perror("send() failed");
                    break;
                }
            }
        }

        if (FD_ISSET(sockfd, &readfds)) {
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

            if (bytes_received == 0) {
                printf("Server disconnected.\n");
                break;
            } else if (bytes_received < 0) {
                perror("recv() failed");
                break;
            } else {
                buffer[bytes_received] = '\0';
                printf("%s", buffer);
            }
        }
    }

    close(sockfd);
    return 0;
}
