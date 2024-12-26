#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

char **get_args(char *mess) {
  char **args = malloc(BUFFER_SIZE * sizeof(char *));
  char *token = strtok(mess, " ");
  int n = 0;

  while (token != NULL) {
    args[n++] = token;
    token = strtok(NULL, " ");
  }
  args[n] = NULL;
  return args;
}

void execute_command(int client_fd) {
  char mess[BUFFER_SIZE];
  ssize_t bytes_read = read(client_fd, mess, BUFFER_SIZE);  

  mess[bytes_read - 1] = '\0';

  pid_t pid = fork();

  if (pid == 0) {
    // Child process
    char **args = get_args(mess);

    // Redirect output to the client
    dup2(client_fd, STDOUT_FILENO); // Redirect stdout to client
    dup2(client_fd, STDERR_FILENO); // Redirect stderr to client

    // Execute the command
    if (execvp(args[0], args) == -1) {
      perror("execvp failed");
    }

    // Free memory and exit if execvp fails
    free(args);
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("Failed to fork");
    return;
  } else {
    // Parent process
    wait(NULL);
  }
}

int main(int argc, char **argv) {
  int server_fd, client_fd;
  struct sockaddr_in serv_addr, cl_addr;
  socklen_t clnt_addr_size = sizeof(cl_addr);

  server_fd = socket(AF_INET, SOCK_STREAM, 0);

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(8000);              // Listen on port 8000
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any IP address

  bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  listen(server_fd, 5);
  printf("Server is listening on port 8000...\n");

  // Accept an incoming connection
  client_fd = accept(server_fd, (struct sockaddr *)&cl_addr, &clnt_addr_size);
  printf("Client connected.\n");

  execute_command(client_fd);

  close(client_fd);
  close(server_fd);

  return 0;
}
