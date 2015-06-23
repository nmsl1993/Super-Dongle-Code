#ifndef UDP_RECEIVER_H

#define UDP_RECEIVER_H
#include <stdint.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#define UDP_PAYLOAD_SIZE 818
#define UDP_PORT 8899
#define CHANNEL_DEPTH 128

typedef struct superdongle_packet{
 uint16_t data[CHANNEL_DEPTH*3];
 uint8_t junk[UDP_PAYLOAD_SIZE - CHANNEL_DEPTH*sizeof(uint16_t)*3];
}superdongle_packet_t;

EXTERNC int loop(char * buffer);
#endif
