
#include <stdio.h>
#include <stdint.h>
#include "kiss_fftr.h"
#include "udp_receiver.h"

#define SAMPLING_FREQUENCY 200e3
typedef struct principal_frequency_t
{
    int index;
    float frequency;
    float psd;
}principal_frequency;
principal_frequency calculate_pf(kiss_fft_cpx * carr, int carr_len);
int main (void) {
  superdongle_packet_t spt __attribute__ ((aligned (__BIGGEST_ALIGNMENT__)));;
  kiss_fftr_cfg fft = kiss_fftr_alloc(CHANNEL_DEPTH,0, 0, 0); //128 input real valued fft, forward direction, let library allocate space
  kiss_fft_scalar rin[CHANNEL_DEPTH];
  kiss_fft_cpx cout[CHANNEL_DEPTH/2 + 1];

  printf("start loop \n");
  while(loop((char *) &spt) != 0)
  {
    int i;
    for(i = 0; i < CHANNEL_DEPTH; i++)
    {
        rin[i] = (kiss_fft_scalar) spt.data[3*i]; //This defaults to float which is correct...
    }

    kiss_fftr(fft,rin,cout);
    cout[0].r = 0;     //Zero meaningless DC component...
    cout[0].i = 0;
    principal_frequency pf = calculate_pf(cout,CHANNEL_DEPTH/2+1);
    printf("%i,%f,%f\n",pf.index,pf.frequency,pf.psd);

  }
  printf("loop done\n");
  return 0;
}
principal_frequency calculate_pf(kiss_fft_cpx * carr, int carr_len)
{
    float mag_sq_sum = 0;
    float mag_sq_max = 0;
    int max_idx;

    int i;
    for(i = 0; i < carr_len; i++)
    {
         float sq_mag = carr[i].r * carr[i].r + carr[i].i * carr[i].i; 
         mag_sq_sum += sq_mag;
         if(sq_mag > mag_sq_max) 
         {
            max_idx = i;
            mag_sq_max = sq_mag;
         }
    }
    principal_frequency pf;
    pf.frequency = (SAMPLING_FREQUENCY)*(float)max_idx/CHANNEL_DEPTH;
    pf.index = max_idx;
    pf.psd = mag_sq_max/mag_sq_sum;

    return pf;
}
 
