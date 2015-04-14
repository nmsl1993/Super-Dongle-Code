#include "main.h"
#include "arm_math.h" 
#include "arm_const_structs.h"
 
 
static arm_rfft_fast_instance_f32 s;

static float32_t rfft_input_chA[SINGLE_CHANNEL_BUFFERSIZE];
static float32_t rfft_input_chB[SINGLE_CHANNEL_BUFFERSIZE];
static float32_t rfft_input_chC[SINGLE_CHANNEL_BUFFERSIZE];

static float32_t rfft_output[SINGLE_CHANNEL_BUFFERSIZE];

static float32_t rfft_mag[SINGLE_CHANNEL_BUFFERSIZE];

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
void do_RFFT(uint16_t * fixed_precision_input) 
{ 
  unsigned int k, max_index;
  float32_t maxValue; 

  for(k = 0; k < SINGLE_CHANNEL_BUFFERSIZE; k++)
  {
  rfft_input_chA[k] = (float32_t)fixed_precision_input[3*k+0]; //Convert uint16_ts to floats.
  rfft_input_chB[k] = (float32_t)fixed_precision_input[3*k+1]; //Convert uint16_ts to floats.
	rfft_input_chC[k] = (float32_t)fixed_precision_input[3*k+2]; //Convert uint16_ts to floats.

  }
 
 //arm_cfft_f32(&arm_cfft_sR_f32_len128, rfft_input_chA, 0, 1);
 //arm_cfft_f32(&arm_cfft_sR_f32_len128, rfft_input_chB, 0, 1);
 //arm_cfft_f32(&arm_cfft_sR_f32_len128, rfft_input_chC, 0, 1);


// arm_rfft_fast_f32(&s,rfft_input_chA,rfft_output,0);
 arm_cmplx_mag_f32(rfft_output, rfft_mag, SINGLE_CHANNEL_BUFFERSIZE);  
 arm_max_f32(rfft_mag, SINGLE_CHANNEL_BUFFERSIZE/2, &maxValue, &max_index); 


 //arm_rfft_fast_f32(&s,rfft_input_chB,rfft_output,0);
 //arm_cmplx_mag_f32(rfft_output, rfft_mag, SINGLE_CHANNEL_BUFFERSIZE);  
 //arm_max_f32(rfft_mag, SINGLE_CHANNEL_BUFFERSIZE/2, &maxValue, &max_index); 

 //arm_rfft_fast_f32(&s,rfft_input_chC,rfft_output,0);
 //arm_cmplx_mag_f32(rfft_output, rfft_mag, SINGLE_CHANNEL_BUFFERSIZE);  
 //arm_max_f32(rfft_mag, SINGLE_CHANNEL_BUFFERSIZE/2, &maxValue, &max_index); 

 //arm_cmplx_mag_f32(rfft_input_chB, rfft_output, SINGLE_CHANNEL_BUFFERSIZE);  
 //arm_max_f32(rfft_output, SINGLE_CHANNEL_BUFFERSIZE, &maxValue, &max_index); 

 //arm_cmplx_mag_f32(rfft_input_chC, rfft_output, SINGLE_CHANNEL_BUFFERSIZE);  
 //arm_max_f32(rfft_output, SINGLE_CHANNEL_BUFFERSIZE, &maxValue, &max_index); 

}
