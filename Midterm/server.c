#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define UDP_PORT 5555
#define MAX_CLIENTS 32
#define RESPONSE_PORT 6666
#define MAX_SIZE 4096

void send_response(char *client_ip, char *message)
{
  //TCP 2nd
  int sock;
  struct sockaddr_in caddr;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror("socket()");
    return;
  }

  caddr.sin_family = AF_INET;
  caddr.sin_port = htons(RESPONSE_PORT);
  inet_pton(AF_INET, client_ip, &caddr.sin_addr);
  
  if (connect(sock, (struct sockaddr *)&caddr, sizeof(caddr)) < 0)
  {
    perror("connect()");
    close(sock);
    return;
  }
  printf("Listening for response channel on port %d...\n", RESPONSE_PORT);
  send(sock, message, strlen(message), 0);
  close(sock);
  printf("Successfully sent response =)))\n");
}

void *handle_request(void *args)
{
  char *data = (char *)args;
  char client_ip[INET_ADDRSTRLEN];
  char *filename, *port_str;
  int data_port;

  sscanf(data, "%s", client_ip);
  char *command = strstr(data, " ") + 1;

  if (strncmp(command, "GET ", 4) != 0)
  {
    send_response(client_ip, "INVALID COMMAND\n");
    free(data);
    return NULL;
  }

  filename = strtok(command + 4, " ");
  port_str = strtok(NULL, " ");
  if (!filename || !port_str || (data_port = atoi(port_str)) <= 0)
  {
    send_response(client_ip, "INVALID COMMAND\n");
    free(data);
    return NULL;
  }

  int data_sock, client_sock;
  struct sockaddr_in saddr, caddr;
  socklen_t client_len = sizeof(caddr);

  //TCP 1st
  data_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (data_sock < 0)
  {
    perror("socket()");
    free(data);
    return NULL;
  }

  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(data_port);
  saddr.sin_addr.s_addr = INADDR_ANY;

  if (bind(data_sock, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
  {
    perror("bind()");
    close(data_sock);
    free(data);
    return NULL;
  }

  if (listen(data_sock, 1) < 0)
  {
    perror("listen()");
    close(data_sock);
    free(data);
    return NULL;
  }

  printf("Listening for data channel on port %d...\n", data_port);
  client_sock = accept(data_sock, (struct sockaddr *)&caddr, &client_len);
  if (client_sock < 0)
  {
    perror("accept()");
    close(data_sock);
    free(data);
    return NULL;
  }

  FILE *file = fopen(filename, "rb");
  if (!file)
  {
    send_response(client_ip, "INVALID COMMAND\n");
    close(client_sock);
    close(data_sock);
    free(data);
    return NULL;
  }

  char buffer[MAX_SIZE];
  int bytes_read;
  while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
  {
    send(client_sock, buffer, bytes_read, 0);
  }

  fclose(file);
  close(client_sock);
  close(data_sock);

  send_response(client_ip, "DONE\n");
  free(data);
  return NULL;
}

int main()
{
  int udp_sock;
  struct sockaddr_in saddr, caddr;
  socklen_t client_len = sizeof(caddr);

  udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (udp_sock < 0)
  {
    perror("socket()");
    exit(EXIT_FAILURE);
  }

  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(UDP_PORT);
  saddr.sin_addr.s_addr = INADDR_ANY;

  if (bind(udp_sock, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
  {
    perror("bind()");
    close(udp_sock);
    exit(EXIT_FAILURE);
  }

  printf("Listening on UDP port %d...\n", UDP_PORT);

  pthread_t threads[MAX_CLIENTS];
  int client_count = 0;

  while (1)
  {
    if (client_count >= MAX_CLIENTS)
    {
      printf("Max clients reached, sleep for 1s...\n");
      sleep(1);
      continue;
    }
    char buffer[1024];
    char client_ip[INET_ADDRSTRLEN];

    int bytes_received = recvfrom(udp_sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&caddr, &client_len);
    if (bytes_received < 0)
    {
      perror("recvfrom()");
      continue;
    }

    buffer[bytes_received] = '\0';
    inet_ntop(AF_INET, &caddr.sin_addr, client_ip, sizeof(client_ip));

    char *request_data = malloc(MAX_SIZE);
    snprintf(request_data, MAX_SIZE, "%s %s", client_ip, buffer);

    if (pthread_create(&threads[client_count], NULL, handle_request, request_data) != 0)
    {
      perror("pthread_create()");
      free(request_data);
      continue;
    }

    client_count++;
  }

  close(udp_sock);
  return 0;
}
