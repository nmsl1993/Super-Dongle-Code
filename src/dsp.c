#include "main.h"
#include "arm_math.h" 
#include "arm_const_structs.h"
#include "dsp.h"
 
static arm_rfft_fast_instance_f32 s;

static float32_t fft_input_chA[SINGLE_CHANNEL_BUFFERSIZE*2];
static float32_t fft_input_chB[SINGLE_CHANNEL_BUFFERSIZE*2];
static float32_t fft_input_chC[SINGLE_CHANNEL_BUFFERSIZE*2];

static float32_t rfft_output[SINGLE_CHANNEL_BUFFERSIZE*2];

static float32_t rfft_mag[SINGLE_CHANNEL_BUFFERSIZE];


static float32_t track_history_cos_chA[PHASE_BUFFER_LEN];
static float32_t track_history_sin_chA[PHASE_BUFFER_LEN];

static float32_t track_history_cos_chB[PHASE_BUFFER_LEN];
static float32_t track_history_sin_chB[PHASE_BUFFER_LEN];

static float32_t track_history_cos_chC[PHASE_BUFFER_LEN];
static float32_t track_history_sin_chC[PHASE_BUFFER_LEN];

static DSP_MODE dsp_mode = SEARCH;

#define CHANNEL_FFT_OFFSET 0

void init_rfft()
{
arm_status status; 
status = ARM_MATH_SUCCESS; 
status = arm_rfft_fast_init_f32(&s, SINGLE_CHANNEL_BUFFERSIZE);
	if(status != ARM_MATH_SUCCESS)
	{
		GPIO_ResetBits(GPIOE,LED3);
	}
}
void do_CFFT(uint16_t * fixed_precision_input,float * freqs) 
{ 
  unsigned int k, max_index1, max_index2, max_index3;
  float32_t maxValue1,maxValue2,maxValue3; 


  for(k = 0; k < SINGLE_CHANNEL_BUFFERSIZE; k++)
  {
  fft_input_chA[2*k] = (float)fixed_precision_input[3*k+0]; //Convert uint16_ts to floats.
  fft_input_chA[2*k+1] = 0;
  fft_input_chB[2*k] = (float)fixed_precision_input[3*k+1]; //Convert uint16_ts to floats.
	fft_input_chB[2*k+1] = 0;
  fft_input_chC[2*k] = (float)fixed_precision_input[3*k+2]; //Convert uint16_ts to floats.
  fft_input_chC[2*k+1] = 0;

  }
 
 arm_cfft_f32(&arm_cfft_sR_f32_len128, fft_input_chA, 0, 1);
 arm_cfft_f32(&arm_cfft_sR_f32_len128, fft_input_chB, 0, 1);
 arm_cfft_f32(&arm_cfft_sR_f32_len128, fft_input_chC, 0, 1);


// arm_rfft_fast_f32(&s,rfft_input_chA,rfft_output,0);

 if(dsp_mode == TRACK)
 {
 arm_cmplx_mag_squared_f32(fft_input_chA, rfft_mag, SINGLE_CHANNEL_BUFFERSIZE);  
 rfft_mag[0] = 0.0;
 arm_max_f32(rfft_mag, SINGLE_CHANNEL_BUFFERSIZE/2, &maxValue1, &max_index1); 

freqs[0] = 100000.0*(float)max_index1/(SINGLE_CHANNEL_BUFFERSIZE/2);
arm_cmplx_mag_squared_f32(fft_input_chB, rfft_mag, SINGLE_CHANNEL_BUFFERSIZE);  
 rfft_mag[0] = 0.0;
 arm_max_f32(rfft_mag, SINGLE_CHANNEL_BUFFERSIZE/2, &maxValue2, &max_index2); 
 freqs[1] = 100000.0*(float)max_index2/(SINGLE_CHANNEL_BUFFERSIZE/2);

arm_cmplx_mag_squared_f32(fft_input_chC, rfft_mag, SINGLE_CHANNEL_BUFFERSIZE);  
  rfft_mag[0] = 0.0;
 arm_max_f32(rfft_mag, SINGLE_CHANNEL_BUFFERSIZE/2, &maxValue3, &max_index3); 
 freqs[2] = 100000.0*(float)max_index3/(SINGLE_CHANNEL_BUFFERSIZE/2);
}
else if(dsp_mode == SEARCH)
  {
    
  }
}
float inline sum_vec(float * arr, int blocksize)
{
    float acc = 0;
    int t;
    for(t = 0; t < blocksize; t++)
    {
      acc += arr[t]; 
    }
    return acc;
}
float circular_var(float * real_vec, float * imag_vec, int blocksize)
{
float cvar = 0;

float real_sum = sum_vec(real_vec,blocksize);
float imag_sum = sum_vec(imag_vec,blocksize);

float r = sqrt(real_sum*real_sum + imag_sum*imag_sum);

float r_normalized = r/blocksize;
//float cos_sumA = sum_vec(track_history_cos_chA,PHASE_BUFFER_LEN);
return 1.0f-r_normalized;
}