

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DSP_H
#define __DSP_H

#ifdef __cplusplus
 extern "C" {
#endif


void init_rfft();
void do_CFFT(uint16_t * fixed_precision_input,float * freqs) ;

#ifdef __cplusplus
}
#endif

#endif /* __DSP_H */



