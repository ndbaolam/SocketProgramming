#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#define MAX_CLIENT 5
#define PORT 8080
#define USERNAME_LEN 32
#define MESSAGE_LEN 1024

typedef struct {
    int fd;
    char username[USERNAME_LEN];
} client_t;

int main(int argc, char** argv) {
    int server_fd, max_fd, activity;
    struct sockaddr_in server_addr, client_addr;
    char buffer[MESSAGE_LEN], message[MESSAGE_LEN + USERNAME_LEN + 5];
    socklen_t addrlen = sizeof(client_addr);
    fd_set readfds;
    client_t client[MAX_CLIENT] = {0}; 

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind()");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen()");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        max_fd = server_fd;

        for (int i = 0; i < MAX_CLIENT; i++) {
            int fd = client[i].fd;
            if (fd > 0) {
                FD_SET(fd, &readfds);                
            }
            if (fd > max_fd) {
                max_fd = fd;
            }
        }

        activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            perror("select()");
        }

        if (FD_ISSET(server_fd, &readfds)) {
            int new_connect = accept(server_fd, (struct sockaddr *) &client_addr, &addrlen);
            if (new_connect < 0) {
                perror("accept()");
                continue;
            }

            char *welcome_msg = "Connected to server!\r\n";
            send(new_connect, welcome_msg, strlen(welcome_msg), 0);

            for (int i = 0; i < MAX_CLIENT; i++) {
                if (client[i].fd == 0) {
                    client[i].fd = new_connect;
                    printf("New connection, socket fd: %d, IP: %s, port: %d\n",
                           new_connect, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENT; i++) {
            int fd = client[i].fd;

            if (fd > 0 && FD_ISSET(fd, &readfds)) {
                int bytes_read = read(fd, buffer, sizeof(buffer));
                if (bytes_read == 0) {
                    printf("User %s disconnected, socket fd: %d\n", client[i].username, fd);
                    close(fd);
                    client[i].fd = 0;
                    memset(client[i].username, 0, USERNAME_LEN);
                } else if (bytes_read < 0) {
                    perror("read()");
                } else {
                    buffer[bytes_read] = '\0';
                    if (strlen(client[i].username) == 0) {
                        strncpy(client[i].username, buffer, USERNAME_LEN - 1);
                        snprintf(buffer, sizeof(buffer), "%s has joined the chat.\r\n", client[i].username);
                        printf("%s", buffer);
                        for (int j = 0; j < MAX_CLIENT; j++) {
                            if (client[j].fd != 0 && client[j].fd != fd) {
                                send(client[j].fd, buffer, strlen(buffer), 0);
                            }
                        }
                    } else {
                        snprintf(message, sizeof(message), "%s: %s\r\n", client[i].username, buffer);                        
                        for (int j = 0; j < MAX_CLIENT; j++) {
                            if (client[j].fd != 0 && client[j].fd != fd) {
                                send(client[j].fd, message, strlen(message), 0);
                            }
                        }
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
