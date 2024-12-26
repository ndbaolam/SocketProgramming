#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

void Append(char **output, const char *str)
{
  char *tmp = *output;
  int oldlen = tmp == NULL ? 0 : strlen(tmp);
  int newlen = oldlen + strlen(str) + 1;
  tmp = realloc(tmp, newlen);
  if (tmp == NULL)
  {
    fprintf(stderr, "Memory allocation failed\n");
    exit(EXIT_FAILURE);
  }
  tmp[newlen - 1] = 0;
  sprintf(tmp + oldlen, "%s", str);
  *output = tmp;
}

void print_folder(char *str, FILE *f, char **content)
{
  struct dirent **namelist;
  int n = scandir(str, &namelist, NULL, alphasort);

  if (n == -1)
  {
    printf("Invalid Folder\n");
    return;
  }

  while (n--)
  {
    char a_tag[1024];
    if (namelist[n]->d_type == DT_DIR)
    {
      // folder
      sprintf(a_tag, "<a href=\"%s\"><b>%s</b></a><br/>\n", namelist[n]->d_name, namelist[n]->d_name);
    }
    else if (namelist[n]->d_type == DT_REG)
    {
      // file
      sprintf(a_tag, "<a href=\"%s\"><i>%s</i></a><br/>\n", namelist[n]->d_name, namelist[n]->d_name);
    }

    Append(content, a_tag);
    free(namelist[n]);
  }

  free(namelist);
}

int main(int argc, char **argv)
{
  char *content = NULL;
  Append(&content, "<html>\n");

  while (1)
  {
    char str[256] = {0};
    printf("Enter a directory path (or type 'exit' to quit): ");
    if (fgets(str, sizeof(str), stdin) == NULL)
    {
      fprintf(stderr, "Error reading input\n");
      break;
    }

    str[strcspn(str, "\n")] = 0;

    if (strcmp(str, "exit") == 0)
      break;

    FILE *f = fopen("output.html", "w");
    if (f == NULL)
    {
      fprintf(stderr, "Failed to open file for writing\n");
      return EXIT_FAILURE;
    }
    free(content);
    content = NULL;
    Append(&content, "<html>\n");
    print_folder(str, f, &content);

    Append(&content, "</html>\n");

    fputs(content, f);
    fclose(f);

    printf("%s\n", content);
  }

  free(content);
  return 0;
}
