#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_CMD_LEN 1024
#define MAX_TOKENS 64
#define DELIM " \t\r\n\a"

void signal_handler(int signum) {
  while(waitpid(-1, NULL, WNOHANG) > 0);
}

char *read_line()
{
  char *line = malloc(MAX_CMD_LEN * sizeof(char));
  fgets(line, sizeof(line), stdin);
  return line;
}

char **split_line(char *line)
{
  int bufsize = MAX_TOKENS, pos = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  token = strtok(line, DELIM);
  while (token != NULL)
  {
    tokens[pos++] = token;
    if (pos >= bufsize)
    {
      bufsize += MAX_TOKENS;
      tokens = realloc(tokens, bufsize * sizeof(char *));
    }
    token = strtok(NULL, DELIM);
  }
  tokens[pos] = NULL;
  return tokens;
}

void execute(char **args)
{  
  pid_t pid = fork();
  if (pid == 0)
  {
    // Child process
    //printf("child pid: %d\n", pid);
    if (execvp(args[0], args) == -1)
    {
      perror("Error executing command");
    }    
    exit(EXIT_FAILURE);
  }
  else if (pid < 0)
  {
    // Fork failed
    perror("Fork failed");
  }
  else
  {    
    wait(NULL); 
    //waitpid(-1, NULL, WNOHANG);
  }
}

int main()
{
  char *line;
  char **agrs;  

  while (1)
  {
    printf("mysh > ");
    line = read_line();
    agrs = split_line(line);

    execute(agrs);

    free(line);
    free(agrs);
  }
  return 0;
}