#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char** argv) {
  int sock, on = 1;
  struct sockaddr_in saddr, baddr;
  socklen_t clen = sizeof(baddr);

  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  //BROADCAST ON
  setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));

  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(8888);
  saddr.sin_addr.s_addr = INADDR_ANY;

  bind(sock, (struct sockaddr *) &saddr, sizeof(saddr));

  baddr.sin_family = AF_INET;
  baddr.sin_port = htons(9999);
  baddr.sin_addr.s_addr = inet_addr("192.168.4.255");

  while(1){
    char buffer[1024] = {0};
    fgets(buffer, sizeof(buffer) - 1, stdin);
    sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &baddr, sizeof(baddr));    
  }

  close(sock);
}