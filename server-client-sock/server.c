#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>

typedef struct {
    char question[200];
    char ans[4][200];
    char correct[200];
} Quest;

void gen_question(Quest q[10]) {
    memset(q, 0, sizeof(Quest) * 10);

    // Define questions and answers
    strcpy(q[0].question, "What is the capital of France?");
    strcpy(q[0].ans[0], "Berlin");
    strcpy(q[0].ans[1], "Madrid");
    strcpy(q[0].ans[2], "Paris");
    strcpy(q[0].ans[3], "Rome");
    strcpy(q[0].correct, "Paris");

    strcpy(q[1].question, "Which planet is known as the Red Planet?");
    strcpy(q[1].ans[0], "Earth");
    strcpy(q[1].ans[1], "Mars");
    strcpy(q[1].ans[2], "Jupiter");
    strcpy(q[1].ans[3], "Saturn");
    strcpy(q[1].correct, "Mars");

    strcpy(q[2].question, "What is the largest ocean on Earth?");
    strcpy(q[2].ans[0], "Atlantic Ocean");
    strcpy(q[2].ans[1], "Indian Ocean");
    strcpy(q[2].ans[2], "Pacific Ocean");
    strcpy(q[2].ans[3], "Arctic Ocean");
    strcpy(q[2].correct, "Pacific Ocean");

    strcpy(q[3].question, "Who wrote the play 'Romeo and Juliet'?");
    strcpy(q[3].ans[0], "William Shakespeare");
    strcpy(q[3].ans[1], "Mark Twain");
    strcpy(q[3].ans[2], "Jane Austen");
    strcpy(q[3].ans[3], "Charles Dickens");
    strcpy(q[3].correct, "William Shakespeare");

    strcpy(q[4].question, "What is the square root of 64?");
    strcpy(q[4].ans[0], "6");
    strcpy(q[4].ans[1], "7");
    strcpy(q[4].ans[2], "8");
    strcpy(q[4].ans[3], "9");
    strcpy(q[4].correct, "8");

    strcpy(q[5].question, "Which element has the chemical symbol 'O'?");
    strcpy(q[5].ans[0], "Gold");
    strcpy(q[5].ans[1], "Silver");
    strcpy(q[5].ans[2], "Oxygen");
    strcpy(q[5].ans[3], "Hydrogen");
    strcpy(q[5].correct, "Oxygen");

    strcpy(q[6].question, "What is the currency of Japan?");
    strcpy(q[6].ans[0], "Dollar");
    strcpy(q[6].ans[1], "Euro");
    strcpy(q[6].ans[2], "Yen");
    strcpy(q[6].ans[3], "Pound");
    strcpy(q[6].correct, "Yen");

    strcpy(q[7].question, "Which sport is known as 'The Beautiful Game'?");
    strcpy(q[7].ans[0], "Basketball");
    strcpy(q[7].ans[1], "Football (Soccer)");
    strcpy(q[7].ans[2], "Tennis");
    strcpy(q[7].ans[3], "Cricket");
    strcpy(q[7].correct, "Football (Soccer)");

    strcpy(q[8].question, "What is the name of the longest river in the world?");
    strcpy(q[8].ans[0], "Nile");
    strcpy(q[8].ans[1], "Amazon");
    strcpy(q[8].ans[2], "Mississippi");
    strcpy(q[8].ans[3], "Yangtze");
    strcpy(q[8].correct, "Nile");

    strcpy(q[9].question, "Which country is known as the 'Land of the Rising Sun'?");
    strcpy(q[9].ans[0], "China");
    strcpy(q[9].ans[1], "Japan");
    strcpy(q[9].ans[2], "South Korea");
    strcpy(q[9].ans[3], "India");
    strcpy(q[9].correct, "Japan");
}

void signchld_handler(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG));
}

void handle_client(int c, Quest q[10]) {    
    int n, points = 0;

    for (int i = 0; i < 10; i++) {
        char formatted_question[1024] = {0};
        char buffer[1024] = { 0 };
        
        sprintf(formatted_question, "Q%d: %s\n1) %s\n2) %s\n3) %s\n4) %s\n",
                i + 1, q[i].question, q[i].ans[0], q[i].ans[1], q[i].ans[2], q[i].ans[3]);

        send(c, formatted_question, strlen(formatted_question), 0);
        
        if ((n = recv(c, buffer, sizeof(buffer), 0)) > 0) {
            buffer[n - 1] = '\0'; 
            printf("Client: %s\n", buffer);

            if (strcmp(q[i].correct, buffer) == 0) {
                points++;
            }
        }
    }

    char score[50];
    sprintf(score, "P: You scored %d out of 10.\n", points);
    send(c, score, strlen(score), 0);
}

int main(int argc, char** argv) {
    int s, c;
    struct sockaddr_in s_addr, c_addr;
    socklen_t clilen = sizeof(c_addr);
    Quest ques[10];
    gen_question(ques);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    s_addr.sin_addr.s_addr = INADDR_ANY;
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(8000);

    if (bind(s, (struct sockaddr *) &s_addr, sizeof(s_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(s, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, signchld_handler); // Handle child processes

    while (1) {
        c = accept(s, (struct sockaddr *) &c_addr, &clilen);
        if (c < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            close(s); 
            handle_client(c, ques);
            exit(EXIT_SUCCESS);
        } else {
            close(c); 
        }
    }

    close(s);
    return 0;
}
