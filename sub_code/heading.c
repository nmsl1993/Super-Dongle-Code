
#include <stdio.h>
#include <stdint.h>
#include "udp_receiver.h"
int main (void) {
  superdongle_packet_t spt __attribute__ ((aligned (__BIGGEST_ALIGNMENT__)));;
  
  printf("start loop \n");
  while(loop((char *) &spt))
  {
    
  }
  printf("loop done\n");
  return 0;
}
