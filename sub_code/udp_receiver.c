/* Note: Sockets, silly. */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "udp_receiver.h"

int sockfd; 

struct sockaddr_in serv_addr;

int bound = 0;

void udp_bind () {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(UDP_PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    int r = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (r == 1) printf("Bind failed!\n");
    else printf("Bound successfully.\n");
}

int loop (superdongle_packet_t * buffer ) {
    if (!bound) { udp_bind(); bound = 1; }

    recvfrom(sockfd, buffer, sizeof(*buffer), 0, NULL, NULL);
    return 0;
}
