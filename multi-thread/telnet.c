#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>

#define PORT 8888

int fd, N = 0;
sem_t mutex;
pthread_t *tid = NULL;
int *g = NULL;

void signal_handler(int signum) {
  if(signum == SIGINT) {
    close(fd);
    for(int i = 0; i < N; i++) {
      pthread_join(tid[i], NULL);
    }
    sem_destroy(&mutex);
    free(tid);
    free(g);
    printf("\nServer closed\n");
    exit(EXIT_SUCCESS);
  }
}

void* handle_client(void *args) {
  int c = *((int *) args);
  free(args);

  char buffer[1024] = {0};
  int bytes_received;

  while ((bytes_received = recv(c, buffer, sizeof(buffer) - 1, 0)) > 0) {
    buffer[bytes_received] = '\0';

    printf("Received command from client %d: %s\n", c, buffer);

    sem_wait(&mutex);
    FILE *file = fopen("command_output.txt", "a");
    if (file == NULL) {
      perror("fopen");
      close(c);
      sem_post(&mutex);
      pthread_exit(NULL);
    }

    time_t now = time(NULL);
    fprintf(file, "\n--- Output from client %d at %s---\n", c, ctime(&now));

    FILE *fp = popen(buffer, "r");
    if (fp == NULL) {
      perror("popen");
      fprintf(file, "Error executing command\n");
      fclose(file);
      sem_post(&mutex);
      close(c);
      pthread_exit(NULL);
    }

    char result[1024];
    size_t bytes_read;

    while ((bytes_read = fread(result, 1, sizeof(result) - 1, fp)) > 0) {
      result[bytes_read] = '\0';
      fprintf(file, "%s", result);
    }

    pclose(fp);
    fclose(file);
    sem_post(&mutex);

    char *msg = "Command output has been appended to command_output.txt\n";
    send(c, msg, strlen(msg), 0);

    memset(buffer, 0, sizeof(buffer));
  }

  close(c);
  pthread_exit(NULL);
}

int main() {
  struct sockaddr_in saddr, caddr;
  socklen_t clen = sizeof(caddr);
  sem_init(&mutex, 0, 1);

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    perror("socket()");
    exit(EXIT_FAILURE);
  }

  signal(SIGINT, signal_handler);

  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  saddr.sin_port = htons(PORT);

  if (bind(fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
    perror("bind()");
    close(fd);
    exit(EXIT_FAILURE);
  }

  if (listen(fd, 5) < 0) {
    perror("listen()");
    close(fd);
    exit(EXIT_FAILURE);
  }

  printf("Server is running on port %d...\n", PORT);

  while (1) {
    int c = accept(fd, (struct sockaddr *) &caddr, &clen);
    if (c < 0) {
      perror("accept");
      continue;
    }

    int *arg = malloc(sizeof(int));
    if (!arg) {
      perror("malloc()");
      close(c);
      continue;
    }
    *arg = c;

    tid = realloc(tid, (N + 1) * sizeof(pthread_t));
    g = realloc(g, (N + 1) * sizeof(int));
    if (!tid || !g) {
      perror("realloc()");
      free(arg);
      close(c);
      continue;
    }

    g[N] = c;
    if (pthread_create(&tid[N], NULL, handle_client, arg) != 0) {
      perror("pthread_create()");
      free(arg);
      close(c);
      continue;
    }

    N++;
  }

  return 0;
}
