#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8000
#define BUFFER_SIZE 1024


void receive_question_and_answer(int sockfd) {    
    int n;
    char buffer[BUFFER_SIZE] = {0};
    char answer[20] = {0};    

    for(int i = 0; i < 10;) {        
        n = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if(n <= 0) return;
        printf("%s", buffer);

        fgets(answer, sizeof(answer), stdin);        
        answer[strcspn(answer, "\n")] = '\0';

        send(sockfd, answer, sizeof(answer), 0);        

        memset(buffer, 0, BUFFER_SIZE);
        memset(answer, 0, sizeof(answer));
    }    

    n = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if(n <= 0) return;
    printf("%s", buffer);
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    receive_question_and_answer(sockfd);

    close(sockfd);
    return 0;
}
