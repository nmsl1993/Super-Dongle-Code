#include "main.h"
#define ARM_MATH_CM4 
#include "arm_math.h" 
#include "arm_const_structs.h"
 
#define FFT_INPUT_LENGTH 2048 
 
/* ------------------------------------------------------------------- 
* External Input and Output buffer Declarations for FFT Bin Example 
* ------------------------------------------------------------------- */ 
extern float32_t rfft_input[FFT_INPUT_LENGTH]; 
static float32_t rfft_output[FFT_INPUT_LENGTH/2]; 
 
/* ------------------------------------------------------------------ 
* Global variables for FFT Bin Example 
* ------------------------------------------------------------------- */ 
uint32_t fftSize = 1024; 
uint32_t ifftFlag = 0; 
uint32_t doBitReverse = 1; 
 

static arm_rfft_fast_instance_f32 s;
/* ---------------------------------------------------------------------- 
* Max magnitude FFT Bin test 
* ------------------------------------------------------------------- */ 
 
void init_rfft()
{
arm_rfft_fast_init_f32(&s, FFT_INPUT_LENGTH);
}
void do_RFFT() 
{ 
   
  arm_status status; 
   
  status = ARM_MATH_SUCCESS; 
   
  /* Process the data through the CFFT/CIFFT module */ 
  arm_rfft_fast_f32(&s, rfft_input, rfft_output, 0);

}