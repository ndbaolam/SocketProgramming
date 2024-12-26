#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKENS 20
#define MAX_BUFFER 1024

char **split_line(char *line) {
  int pos = 0;
  char **tokens = malloc(MAX_TOKENS * sizeof(char *));
  char *token;

  token = strtok(line, " ");
  while (token != NULL) {
    tokens[pos++] = token;    
    token = strtok(NULL, " ");
  }
  tokens[pos] = NULL;
  return tokens;
}

int main() {
  FILE *f;
  char buf[MAX_BUFFER];
  char **line;

  system("ifconfig | grep inet > output.txt");
  f = fopen("output.txt", "rt");

  while(!feof(f)) {    
    fgets(buf, sizeof(buf) - 1, f);        
    line = split_line(buf);
    printf("%s\n", line[1]);

    free(line);
  }

  fclose(f);
  return 0;
}