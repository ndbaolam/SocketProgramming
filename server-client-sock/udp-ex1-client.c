#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void generate_random_name(char *name, int length) {
    srand(time(NULL));
    int num1 = rand() % 10;
    int num2 = rand() % 10;
    int num3 = rand() % 10;
    int num4 = rand() % 10;
    snprintf(name, length, "uname %d%d%d%d", num1, num2, num3, num4);
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[BUFFER_SIZE];
    char name[50];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    generate_random_name(name, sizeof(name));

    snprintf(buffer, sizeof(buffer), "REG %s", name);
    sendto(sockfd, buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Broadcast: %s\n", buffer);

    while (1) {
        printf("Enter command (LIST to get client list): ");
        char command[10];
        scanf("%s", command);

        if (strcmp(command, "LIST") == 0) {
            snprintf(buffer, sizeof(buffer), "LIST");
            sendto(sockfd, buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));

            int n = recvfrom(sockfd, buffer, BUFFER_SIZE, MSG_WAITALL, NULL, NULL);
            buffer[n] = '\0';
            printf("Clients in network:\n%s\n", buffer);
        }
    }

    close(sockfd);
    return 0;
}
