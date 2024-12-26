#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  struct dirent **nameList;

  int n = scandir(".", &nameList, NULL, NULL);

  if(n < 0) {
    perror("Error");
    return 1;
  }

  while(n--) {
    printf("filename: %s, type: %c\n", nameList[n]->d_name, nameList[n]->d_type);
    free(nameList[n]);
  }

  free(nameList);
  return 0;
}
