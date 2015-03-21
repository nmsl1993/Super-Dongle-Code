#include "main.h"
#include "arm_math.h" 
#include "arm_const_structs.h"
 
 
 
/* ------------------------------------------------------------------- 

* External Input and Output buffer Declarations for FFT Bin Example 
* ------------------------------------------------------------------- */
static float32_t rfft_input[SINGLE_CHANNEL_BUFFERSIZE*2];
static float32_t rfft_output[SINGLE_CHANNEL_BUFFERSIZE]; 


void init_rfft()
{
//arm_status status; 
//status = ARM_MATH_SUCCESS; 
//status = arm_cfft_f32(&s, SINGLE_CHANNEL_BUFFERSIZE);
//if(status != ARM_MATH_SUCCESS)
//	{
//		GPIO_ResetBits(GPIOE,LED3);
//	}
}
void do_RFFT(uint16_t * fixed_precision_input) 
{ 
  unsigned int k, max_index;
  float32_t maxValue; 

  for(k = 0; k < SINGLE_CHANNEL_BUFFERSIZE; k++)
  {
    rfft_input[k] = (float32_t)fixed_precision_input[k]; //Convert uint16_ts to floats.
  }
 
 arm_cfft_f32(&arm_cfft_sR_f32_len128, rfft_input, 0, 1);
 //arm_cmplx_mag_f32(rfft_input, rfft_output, SINGLE_CHANNEL_BUFFERSIZE);  

  //arm_max_f32(rfft_output, SINGLE_CHANNEL_BUFFERSIZE, &maxValue, &max_index); 

}
