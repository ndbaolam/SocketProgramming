#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define MAXLINE 1024
#define KEY 69

char *message_decryption(char *encrypted, char key) {
    int len = strlen(encrypted) - 1; 
    char *decrypt = malloc(len + 1);

    if (decrypt == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    char iv = encrypted[0];

    for (int i = 0; i < len; i++) {
        decrypt[i] = encrypted[i + 1] ^ key ^ iv; 
    }

    decrypt[len] = '\0';
    return decrypt;
}

int main() {
    int sockfd;
    char buffer[MAXLINE];
    char *message = "Hello, Server!";
    struct sockaddr_in servaddr, recvaddr;
    socklen_t len;
    fd_set readfds;
    struct timeval timeout;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&recvaddr, 0, sizeof(recvaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Connect failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        
        timeout.tv_sec = 10; 
        timeout.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        int activity = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0) {
            perror("select()");
            continue;
        } else if (activity == 0) {
            printf("Timeout, no response from server.\n");
        } else {
            len = sizeof(servaddr);
            char recv_ip[INET_ADDRSTRLEN];
            char input[MAXLINE] = {0};
            int n = recvfrom(sockfd, buffer, MAXLINE - 1, 0, (struct sockaddr *) &recvaddr, &len);

            if (n < 0) {
                perror("recvfrom()");
                continue;
            }
            
            inet_ntop(AF_INET, &recvaddr.sin_addr, recv_ip, INET_ADDRSTRLEN);

            if (memcmp(&servaddr, &recvaddr, sizeof(struct sockaddr_in)) == 0) {
                printf("Received message from the correct server (IP: %s, Port: %d).\n", recv_ip, ntohs(recvaddr.sin_port));
                printf("Server message: %s\n", buffer);
            } else {
                printf("Received message from an unexpected server (IP: %s, Port: %d).\n", recv_ip, ntohs(recvaddr.sin_port));
            }

            buffer[n] = '\0'; 
            char *decrypted = message_decryption(buffer, KEY);
            printf("Decrypted: %s\n", decrypted);

            printf("Verify server: ");
            fgets(input, MAXLINE - 1, stdin);
            sendto(sockfd, input, strlen(input), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));

            free(decrypted);
            break;
        }        
    }

    close(sockfd);
    return 0;
}
