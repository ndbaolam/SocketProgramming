#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

typedef struct {
    struct sockaddr_in caddr;
    char *username;
} USER;

USER *gaddr = NULL;
int g_count = 0;

void Append(char** output, const char* str) {
    char* tmp = *output;
    int oldlen = tmp == NULL ? 0 : strlen(tmp);
    int newlen = oldlen + strlen(str) + 1;
    tmp = realloc(tmp, newlen);
    if (tmp == NULL) {
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    *output = tmp;
    strcpy(tmp + oldlen, str);
}

char *getUserName(char *buffer) {  
    char *token = strtok(buffer, " ");
    if (token == NULL) {
        perror("strtok()");
        exit(EXIT_FAILURE);
    }
    token = strtok(NULL, " ");
    if (token == NULL) {
        perror("strtok()");
        exit(EXIT_FAILURE);
    }
    token = strtok(NULL, " ");
    if (token == NULL) {
        perror("strtok()");
        exit(EXIT_FAILURE);
    }
    return strdup(token); // Sao chép chuỗi để lưu trữ
}

int isConnect(struct sockaddr_in *caddr) {
    for (int i = 0; i < g_count; i++) {
        if (gaddr[i].caddr.sin_addr.s_addr == caddr->sin_addr.s_addr) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    int s, on = 1; 
    struct sockaddr_in saddr, baddr, caddr;
    socklen_t clen = sizeof(caddr);

    // Tạo socket và kích hoạt chế độ broadcast
    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

    // Cấu hình địa chỉ của server
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(8080);

    // Ràng buộc socket với địa chỉ server
    if (bind(s, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
        perror("bind()");
        close(s);
        exit(EXIT_FAILURE);
    }

    // Cấu hình địa chỉ broadcast
    baddr.sin_family = AF_INET;
    baddr.sin_port = htons(9999);  // Cổng cho broadcast
    baddr.sin_addr.s_addr = inet_addr("192.168.4.255");  // Địa chỉ broadcast

    while (1) {
        char buffer[1024] = {0};
        memset(&caddr, 0, sizeof(caddr));
        recvfrom(s, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *) &caddr, &clen);

        if (strncmp("REG", buffer, 3) == 0) {
            if (!isConnect(&caddr)) {
                gaddr = realloc(gaddr, (g_count + 1) * sizeof(USER));
                if (gaddr == NULL) {
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }
                gaddr[g_count].username = getUserName(buffer);
                memcpy(&gaddr[g_count].caddr, &caddr, sizeof(struct sockaddr_in));
                g_count++;
                printf("New user registered: %s\n", gaddr[g_count-1].username);
            }
        } else if (strncmp("LIST", buffer, 4) == 0) {
            char *mess = strdup("CLIENTS N\n");
            for (int i = 0; i < g_count; i++) {
                char temp[256] = {0};
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(gaddr[i].caddr.sin_addr), client_ip, INET_ADDRSTRLEN);
                sprintf(temp, "%s %s\n", gaddr[i].username, client_ip);
                Append(&mess, temp);
            }

            // Gửi danh sách client tới tất cả các client qua broadcast
            sendto(s, mess, strlen(mess), 0, (struct sockaddr *) &baddr, sizeof(baddr));
            free(mess);  // Giải phóng bộ nhớ đã phân bổ
        }
    }

    close(s);
}
