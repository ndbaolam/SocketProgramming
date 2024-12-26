#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/select.h>
#include <time.h>

#define PORT 8080
#define MAXLINE 1024
#define MAX_CLIENTS 100
#define INET_ADDRSTRLEN 16
#define KEY 69

int sockfd;

void signal_handler(int signum) {
    close(sockfd);
    printf("\nServer closed.\n");
    exit(EXIT_SUCCESS);
}

char *message_encryption(char *data, char key) {
    int len = strlen(data);
    char *encrypt = malloc(len + 2);

    if (encrypt == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    char iv = rand() % 256;

    encrypt[0] = iv;

    for (int i = 0; i < len; i++) {
        encrypt[i + 1] = data[i] ^ key ^ iv; 
    }

    encrypt[len + 1] = '\0';
    return encrypt;
}

char *generate_message(int size) {
    char *mess = malloc(size + 1);
    srand(time(NULL));
    int i;

    for (i = 0; i < size; i++) {
        mess[i] = 'a' + rand() % 26;
    }

    mess[i] = '\0';

    return mess;
}

int main() {
    struct sockaddr_in sv_addr, cl_addr;
    socklen_t len = sizeof(cl_addr);
    fd_set readfds;    
    int max_fd, act;
    struct timeval timeout;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }

    memset(&sv_addr, 0, sizeof(sv_addr));
    sv_addr.sin_family = AF_INET;
    sv_addr.sin_addr.s_addr = INADDR_ANY;
    sv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&sv_addr, sizeof(sv_addr)) < 0) {
        perror("bind()");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP server is running and listening on port %d...\n", PORT);
    signal(SIGINT, signal_handler);

    while (1) {
        char buffer[MAXLINE];
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        max_fd = sockfd;  
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;      

        char *message = generate_message(16);
        char *encrypted = message_encryption(message, KEY);
        int n;

        n = recvfrom(sockfd, buffer, MAXLINE - 1, 0, (struct sockaddr *)&cl_addr, &len);
        if (n < 0) {
            perror("recvfrom()");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cl_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Sent to %s:%d\nMessage: %s\n", client_ip, ntohs(cl_addr.sin_port), message);

        n = sendto(sockfd, encrypted, strlen(encrypted), 0, (struct sockaddr *)&cl_addr, len);
        if (n < 0) {
            perror("sendto()");
            continue;
        }                

        act = select(max_fd + 1, &readfds, NULL, NULL, &timeout);

        if (act < 0) {
            perror("select()");            
        } else if (act == 0) {
            printf("Timeout: %s:%d\n", client_ip, ntohs(cl_addr.sin_port));
        } else {
            if (FD_ISSET(sockfd, &readfds)) {
                memset(buffer, 0, sizeof(buffer));
                n = recvfrom(sockfd, buffer, MAXLINE - 1, 0, (struct sockaddr *)&cl_addr, &len);
                printf("Verify: %s\n", buffer);
            }
        }

        free(message);
        free(encrypted);
    }

    close(sockfd);
    return 0;
}
