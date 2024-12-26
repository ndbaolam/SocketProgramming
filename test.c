#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <dirent.h>

#define PORT 8888

void Append(char **output, const char *str)
{
  char *tmp = *output;
  int oldlen = tmp ? strlen(tmp) : 0;
  int newlen = oldlen + strlen(str) + 1;

  tmp = realloc(tmp, newlen);
  if (tmp == NULL)
  {
    fprintf(stderr, "Memory allocation failed.\n");
    return;
  }

  strcpy(tmp + oldlen, str);

  *output = tmp;
}

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

char *HandleRequest(int client_fd) {
  char *req = NULL;
  char ch;
  int n = 0;

  while (recv(client_fd, &ch, 1, 0) > 0) {
    char *temp = realloc(req, n + 2);  // Allocate space for the new char and null terminator
    if (temp == NULL) {
      free(req);  // Free memory if realloc fails
      return NULL;
    }
    req = temp;
    req[n++] = ch;

    if (strstr(req, "\r\n\r\n")) {  // Check for end of HTTP header
      break;
    }
  }

  if (req) {
    req[n] = '\0';  // Null-terminate the request
  }

  return req;
}


int main(int argc, char **argv)
{
  int fd, act, cd, max_sd;
  struct sockaddr_in saddr, caddr;
  socklen_t clen = sizeof(caddr);
  fd_set readfds;
  struct dirent *entry;

  if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  saddr.sin_port = htons(PORT);

  if (bind(fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
  {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  printf("Listener on port %d \n", PORT);

  if (listen(fd, 3) < 0)
  {
    perror("listen");
    exit(EXIT_FAILURE);
  }
  puts("Waiting for connections ...");
  if ((cd = accept(fd, (struct sockaddr *)&caddr, &clen)) < 0)
  {
    perror("accept error");
    exit(EXIT_FAILURE);
  }

  char *req = HandleRequest(cd);
  
  printf("%s\n", req);

  char *res = "HTTP1/1 200 OK\r\nContent-Length: 0\r\n\r\n";

  send(cd, res, strlen(res), 0);
  
  free(req);  
  close(fd);
  close(cd);

  return 0;
}
