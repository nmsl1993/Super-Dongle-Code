

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4x7_eth_bsp.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define LED1 GPIO_Pin_9
#define LED2 GPIO_Pin_11
#define LED3 GPIO_Pin_13
#define LED4 GPIO_Pin_15
#define PGA0 GPIO_Pin_3
#define PGA1 GPIO_Pin_2

#define DEST_IP_ADDR0   10
#define DEST_IP_ADDR1   0
#define DEST_IP_ADDR2   0
#define DEST_IP_ADDR3   255
 
#define UDP_SERVER_DATA_PORT    8899   //Local port for streaming data
#define UDP_CLIENT_DATA_PORT    8899   //Remote port for streaming data
#define UDP_SERVER_COMMAND_PORT    8898   //Local port for communications
#define UDP_CLIENT_COMMAND_PORT    8898   //Remote port for communications

/* MAC ADDRESS: MAC_ADDR0:MAC_ADDR1:MAC_ADDR2:MAC_ADDR3:MAC_ADDR4:MAC_ADDR5 */
#define MAC_ADDR0   0xAA
#define MAC_ADDR1   0xAA
#define MAC_ADDR2   0xAA
#define MAC_ADDR3   0xAA
#define MAC_ADDR4   0xAA
#define MAC_ADDR5   0xFF
 
/*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0   10
#define IP_ADDR1   0
#define IP_ADDR2   0
#define IP_ADDR3   111
   
/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   10
#define GW_ADDR1   0
#define GW_ADDR2   0
#define GW_ADDR3   1

/* MII and RMII mode selection, for STM324xG-EVAL Board(MB786) RevB ***********/
#define RMII_MODE  // User have to provide the 50 MHz clock by soldering a 50 MHz
                     // oscillator (ref SM7745HEV-50.0M or equivalent) on the U3
                     // footprint located under CN3 and also removing jumper on JP5. 
                     // This oscillator is not provided with the board. 
                     // For more details, please refer to STM3240G-EVAL evaluation
                     // board User manual (UM1461).

                                     
//#define MII_MODE

/* Uncomment the define below to clock the PHY from external 25MHz crystal (only for MII mode) */
#ifdef 	MII_MODE
 #define PHY_CLOCK_MCO
#endif
#define MYROC_PKT_PAYLOAD_LEN 19
/* STM324xG-EVAL jumpers setting
    +==========================================================================================+
    +  Jumper |       MII mode configuration            |      RMII mode configuration         +
    +==========================================================================================+
    +  JP5    | 2-3 provide 25MHz clock by MCO(PA8)     |  Not fitted                          +
    +         | 1-2 provide 25MHz clock by ext. Crystal |                                      +
    + -----------------------------------------------------------------------------------------+
    +  JP6    |          2-3                            |  1-2                                 +
    + -----------------------------------------------------------------------------------------+
    +  JP8    |          Open                           |  Close                               +
    +==========================================================================================+
  */
   
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */  
void Time_Update(void);
void Delay(uint32_t nCount);
extern struct pbuf *p;
extern __IO  uint32_t message_count;
extern struct udp_pcb *upcb;

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

