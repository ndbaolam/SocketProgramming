#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <malloc.h>
#include <sys/select.h>
#include <dirent.h>
#include <sys/stat.h> 

#define SOCKADDR_IN struct sockaddr_in
#define SOCKADDR struct sockaddr
char curPath[2048] = { 0 };
char* html = NULL;

void Append(char** output, const char* str)
{
    char* tmp = *output;
    int oldlen = tmp == NULL ? 0 : strlen(tmp);
    int newlen = oldlen + strlen(str) + 1;
    tmp = realloc(tmp, newlen);
    tmp[newlen - 1] = 0;
    sprintf(tmp + oldlen, "%s", str);
    *output = tmp;
}

void Send(int c, char* data, int len)
{
    int sent = 0;
    while (sent < len)
    {
        int s = send(c, data + sent, len - sent, 0);
        if (s > 0)
        {
            sent += s;
        }
        else
            break;
    }
}

int Compare(const struct dirent** item1, const struct dirent** item2)
{
    if ((*item1)->d_type == DT_DIR)
        return 1;
    else if ((*item2)->d_type == DT_DIR)
        return -1;
    else
        return 0;
}

void* ClientThread(void* arg)
{
    int c = *((int*)arg);
    free(arg);
    arg = NULL;
    char* buffer = NULL;
    int len = 0;
    while (1)
    {
        char chr;
        int r = recv(c, &chr, 1, 0);
        if (r > 0)
        {
            buffer = (char*)realloc(buffer, len + 1);
            buffer[len] = chr;
            len++;
            if (strstr(buffer, "\r\n\r\n") != NULL)
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    if (strstr(buffer, "\r\n\r\n") != NULL)
    {
        char action[16] = { 0 };
        char path[1024] = { 0 };
        sscanf(buffer, "%s%s", action, path);

        char* found = strstr(path, "%20");
        while (found != NULL)
        {
            *found = ' ';
            memmove(found + 1, found + 3, strlen(found + 3) + 1);
            found = strstr(path, "%20");
        }

        if (strcmp(action, "GET") == 0)
        {
            if (strcmp(path, "/") == 0)
            {
                strcpy(curPath, "/");
            }
            else
            {
                sprintf(curPath, "%s", path);
            }

            struct stat pathStat;
            stat(curPath, &pathStat);

            if (S_ISDIR(pathStat.st_mode))
            {
                Append(&html, "<html><body>");
                Append(&html, "<h1>Directory Listing</h1>");

                struct dirent** namelist;
                int n = scandir(curPath, &namelist, NULL, Compare);
                if (n < 0)
                {
                    Append(&html, "Failed to open directory");
                }
                else
                {
                    while (n--)
                    {
                        if (namelist[n]->d_type == DT_DIR)
                        {
                            Append(&html, "<a href=\"");
                            Append(&html, path);
                            if (path[strlen(path) - 1] != '/')
                                Append(&html, "/");
                            Append(&html, namelist[n]->d_name);
                            Append(&html, "/\">");
                            Append(&html, "<b>");
                            Append(&html, namelist[n]->d_name);
                            Append(&html, "</b></a><br>");
                        }
                        else
                        {
                            Append(&html, "<a href=\"");
                            Append(&html, path);
                            if (path[strlen(path) - 1] != '/')
                                Append(&html, "/");
                            Append(&html, namelist[n]->d_name);
                            Append(&html, "?\">");
                            Append(&html, namelist[n]->d_name);
                            Append(&html, "</a><br>");
                        }
                        free(namelist[n]);
                    }
                    free(namelist);
                }

                Append(&html, "</body></html>");

                char* ok = "HTTP/1.1 200 OK\r\n\r\n";
                Send(c, ok, strlen(ok));
                Send(c, html, strlen(html));
                free(html);
                html = NULL;
            }
            else
            {
                FILE* f = fopen(curPath, "rb");
                if (f == NULL)
                {
                    char* notFound = "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
                    Send(c, notFound, strlen(notFound));
                }
                else
                {
                    fseek(f, 0, SEEK_END);
                    int fsize = ftell(f);
                    fseek(f, 0, SEEK_SET);
                    char* fileData = (char*)malloc(fsize);
                    int read = 0;
                    while (read < fsize)
                    {
                        int tmp = fread(fileData + read, 1, fsize - read, f);
                        read += tmp;
                    }
                    fclose(f);

                    char* ok = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\n\r\n";
                    Send(c, ok, strlen(ok));
                    Send(c, fileData, fsize);
                    free(fileData);
                }
            }
        }
    }

    close(c);
    free(buffer);
}

int main()
{
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    saddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    if (bind(s, (SOCKADDR*)&saddr, sizeof(saddr)) == 0)
    {
        listen(s, 10);
        while (1)
        {
            int clen = sizeof(caddr);
            int c = accept(s, (SOCKADDR*)&caddr, &clen);
            pthread_t tid;
            int* arg = (int*)calloc(1, sizeof(int));
            *arg = c;
            pthread_create(&tid, NULL, ClientThread, arg);
        }
    }
    else
    {
        printf("Failed to bind\n");
        close(s);
    }
}
