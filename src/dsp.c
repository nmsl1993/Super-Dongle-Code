#include "main.h"
#include "arm_math.h" 
#include "arm_const_structs.h"
 
 
 
/* ------------------------------------------------------------------- 

* External Input and Output buffer Declarations for FFT Bin Example 
* ------------------------------------------------------------------- */
static float32_t rfft_input[SINGLE_CHANNEL_BUFFERSIZE];
static float32_t rfft_output[SINGLE_CHANNEL_BUFFERSIZE]; 


static arm_rfft_fast_instance_f32 s;
/* ---------------------------------------------------------------------- 
* Max magnitude FFT Bin test 
* ------------------------------------------------------------------- */ 
 
void init_rfft()
{
arm_rfft_fast_init_f32(&s, SINGLE_CHANNEL_BUFFERSIZE);
}
void do_RFFT(uint16_t * fixed_precision_input) 
{ 
  unsigned int k;
  for(k = 0; k < SINGLE_CHANNEL_BUFFERSIZE; k++)
  {
    rfft_input[k] = (float32_t)fixed_precision_input[k]; //Convert uint16_ts to floats.
  }
  arm_status status; 
   
  status = ARM_MATH_SUCCESS; 
   
  /* Process the data through the CFFT/CIFFT module */ 
  arm_rfft_fast_f32(&s, rfft_input, rfft_output, 0);

}
