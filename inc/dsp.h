

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DSP_H
#define __DSP_H

#ifdef __cplusplus
 extern "C" {
#endif

#define PHASE_BUFFER_LEN 32

typedef enum {TRACK=0,SEARCH=1} DSP_MODE; 
void init_rfft();
void do_CFFT(uint16_t * fixed_precision_input,float * freqs) ;

#ifdef __cplusplus
}
#endif

#endif /* __DSP_H */



