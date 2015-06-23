/* Note: Sockets, silly. */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define UDP_PAYLOAD_SIZE 818
#define UDP_PORT 8899

int sockfd; 
char buf[UDP_PAYLOAD_SIZE];

struct sockaddr_in serv_addr, cli_addr;

void loop() {
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(UDP_PORT);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  int r = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
  if (r == 1) printf("Bind failed!\n");
  else printf("Bound successfully.\n");
  
  while (1) {
    struct sockaddr_storage src_addr;
    socklen_t src_addr_len=sizeof(src_addr);
    ssize_t count=recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&src_addr, &src_addr_len);

    // "buf" holds data received over UDP.
    printf("RECV: %s\n", buf);
  }
}

// Just for testing.
int main (void) {
  loop();
  return 0;
}