
#include <stdio.h>
#include <stdint.h>
#include "udp_receiver.h"
int main (void) {
superdongle_packet_t spt __attribute__ ((aligned (__BIGGEST_ALIGNMENT__)));;
  loop((char *) &spt);
  return 0;
}
