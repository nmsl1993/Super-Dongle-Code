
#include <stdio.h>
#include <stdint.h>
#include "kiss_fftr.h"
#include "udp_receiver.h"
#include <math.h>
#define SAMPLING_FREQUENCY 100e3
#define PSD_THRESHOLD .50000f
//#define PSD_THRESHOLD 0.0f
typedef struct principal_frequency_t
{
    int index;
    float frequency;
    kiss_fft_cpx dft_term;
    float psd;
}principal_frequency;

principal_frequency calculate_pf(kiss_fft_cpx * carr, int carr_len);
kiss_fft_cpx calculate_single_dft_term(int index, kiss_fft_scalar * rarr, int rarr_len);
float phase_difference(float phase_a, float phase_b);
int main (void) {
  superdongle_packet_t spt __attribute__ ((aligned (__BIGGEST_ALIGNMENT__)));;
  kiss_fftr_cfg fft = kiss_fftr_alloc(CHANNEL_DEPTH,0, 0, 0); //128 input real valued fft, forward direction, let library allocate space
  kiss_fft_scalar rinA[CHANNEL_DEPTH];
  kiss_fft_scalar rinB[CHANNEL_DEPTH];
  kiss_fft_scalar rinC[CHANNEL_DEPTH];
  kiss_fft_cpx cout[CHANNEL_DEPTH/2 + 1];

  printf("start loop \n");
  int bufcount = 0;
  while(loop(&spt) == 0)
  {
    int i;

    //printf("DATA\n");
    for(i = 0; i < CHANNEL_DEPTH; i++)
    {
      //  printf("%i\n",spt.data[3*i]);
        rinA[i] = (kiss_fft_scalar) spt.data[3*i]; //This defaults to float which is correct...
        rinB[i] = (kiss_fft_scalar) spt.data[3*i+1]; //This defaults to float which is correct...
        rinC[i] = (kiss_fft_scalar) spt.data[3*i+2]; //This defaults to float which is correct...
        //printf("%f\n", rin[i]);
    }
    //printf("END DATA\n");

    kiss_fftr(fft,rinA,cout);
    cout[0].r = 0;     //Zero meaningless DC component...
    cout[0].i = 0;
    principal_frequency pf = calculate_pf(cout,CHANNEL_DEPTH/2+1);
    if(pf.psd >= PSD_THRESHOLD)
    {
     
    printf("%i,%f,%f\n",pf.index,pf.frequency,pf.psd);
    kiss_fft_cpx TA_X_k = pf.dft_term;
    kiss_fft_cpx TB_X_k = calculate_single_dft_term(pf.index,rinB,CHANNEL_DEPTH);
    kiss_fft_cpx TC_X_k = calculate_single_dft_term(pf.index,rinC,CHANNEL_DEPTH);
    //printf("TA: %f+%fj\n", TA_X_k.r,TA_X_k.i);
    //printf("TB: %f+%fj\n", TB_X_k.r,TB_X_k.i);
    //printf("TC: %f+%fj\n", TC_X_k.r,TC_X_k.i);
   
    float TA_phase = atan2(TA_X_k.i,TA_X_k.r);
    float TB_phase = atan2(TB_X_k.i,TB_X_k.r);
    float TC_phase = atan2(TC_X_k.i,TC_X_k.r);

    float delta_phase_Y = phase_difference(TC_phase,TB_phase);
    float delta_phase_X = phase_difference(TA_phase,TB_phase);
    float heading = atan2(delta_phase_Y,delta_phase_X);

    printf("Index %i, Heading in degrees: %f\n",bufcount, heading*180/M_PI); 
    }
    bufcount++;
  }
  printf("loop done\n");
  return 0;
}
float phase_difference(float phase_a, float phase_b)
{
    float diff = phase_a - phase_b;
    while(diff > M_PI)
    {
        diff -= 2*M_PI;
    }
    while(diff < -1*M_PI)
    {
        diff += 2*M_PI;
    }
    return diff;
}
kiss_fft_cpx calculate_single_dft_term(int index, kiss_fft_scalar * rarr, int rarr_len)
{
    int k = index;
    float N = rarr_len;
    int n;
    
    kiss_fft_cpx X_k;
    X_k.r = 0;
    X_k.i = 0;
    for(n = 0; n < rarr_len; n++)
    {
    kiss_fft_scalar theta = -2*M_PI*k*n/N;
    kiss_fft_cpx complex_multiplier;
    complex_multiplier.r = cos(theta)*rarr[n];
    complex_multiplier.i = sin(theta)*rarr[n];
    
    X_k.r += complex_multiplier.r;
    X_k.i += complex_multiplier.i;
    }
    //X_k.r *= 1/sqrt(N);
    //X_k.i *= 1/sqrt(N);
    return X_k;
}
principal_frequency calculate_pf(kiss_fft_cpx * carr, int carr_len)
{
    float mag_sq_sum = 0;
    float mag_sq_max = 0;
    int max_idx = 0;

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
    pf.dft_term = carr[max_idx];
    pf.psd = mag_sq_max/mag_sq_sum;
    
    return pf;
}
 
