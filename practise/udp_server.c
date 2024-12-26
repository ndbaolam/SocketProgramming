#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char** argv) {
    int sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t cli_len;
    char buffer[255];
    int n;

    // Create a socket (IPv4, Datagram, UDP)
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        error("Error opening socket");
    }

    // Zero out the server address structure
    memset(&serv_addr, 0, sizeof(serv_addr));

    // Set up the server address
    serv_addr.sin_family = AF_INET; // IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY; // Any incoming interface
    serv_addr.sin_port = htons(8080); // Port number

    // Bind the socket to the address and port
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("Error on binding");
    }

    cli_len = sizeof(cli_addr);

    // Receive a message from the client
    n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &cli_addr, &cli_len);
    if (n < 0) {
        error("Error on recvfrom");
    }

    // Null-terminate the received data and print it
    buffer[n] = '\0';
    printf("Client: %s\n", buffer);

    // Send a response to the client
    const char *response = "Message received";
    n = sendto(sockfd, response, strlen(response), 0, (struct sockaddr *) &cli_addr, cli_len);
    if (n < 0) {
        error("Error on sendto");
    }

    // Close the socket
    close(sockfd);
    return 0;
}
