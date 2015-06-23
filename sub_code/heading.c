
#include <stdio.h>
#include <stdint.h>
#include "kiss_fftr.h"
#include "udp_receiver.h"

#define SAMPLING_FREQUENCY 200e3
typedef struct principal_frequency_t
{
    float frequency;
    int index;
    float phase_noise;
}principal_frequency;

int main (void) {
  superdongle_packet_t spt __attribute__ ((aligned (__BIGGEST_ALIGNMENT__)));;
  kiss_fftr_cfg fft = kiss_fftr_alloc(CHANNEL_DEPTH,0, 0, 0); //128 input real valued fft, forward direction, let library allocate space
  kiss_fft_scalar rin[CHANNEL_DEPTH];
  kiss_fft_cpx sout[CHANNEL_DEPTH/2 + 1];

  printf("start loop \n");
  while(loop((char *) &spt))
  {
    int i;
    for(i = 0; i < CHANNEL_DEPTH; i++)
    {
        rin[i] = (kiss_fft_scalar) spt.data[3*i]; //This defaults to float which is correct...
    }

    kiss_fftr(fft,rin,sout);


  }
  printf("loop done\n");
  return 0;
}

 
