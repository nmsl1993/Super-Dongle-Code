

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DSP_H
#define __DSP_H

#ifdef __cplusplus
 extern "C" {
#endif


void init_rfft();
void do_RFFT(uint16_t * fixed_precision_input) ;

#ifdef __cplusplus
}
#endif

#endif /* __DSP_H */



