
// STM32 ADC IQ Sample @ 200 KHz (PC.1, PC.2) STM32F4 Discovery - sourcer32@gmail.com

// Assumptions per system_stm32f4xx.c CPU @ 168 MHz, APB2 @ 84 MHz (/2), APB1 @ 42 MHz (/4)
#include "stm32f4x7_eth.h"
#include "misc.h"
#include "stm32f4xx_conf.h"
#include "netconf.h"
#include "main.h"

#include "httpd.h"
#include "ip_addr.h"

#define SYSTEMTICK_PERIOD_MS  10
#define BUFFERSIZE 300  // I+Q 200KHz x2 HT/TC at 1KHz
#define ADC_CDR_ADDRESS    ((uint32_t)0x40012308)
RCC_ClocksTypeDef RCC_Clocks;

__IO uint16_t ADCTripleConvertedValues[BUFFERSIZE]; // Filled as pairs ADC1, ADC2
__IO uint16_t ADCTripleConvertedValuesShadow[BUFFERSIZE]; // Filled as pairs ADC1, ADC2
__IO uint32_t LocalTime = 0; /* this variable is used to create a time reference incremented by 10ms */

volatile uint8_t doADCTransfer=0;
uint32_t timingdelay;
struct udp_pcb *upcb;
__IO   uint32_t message_count = 0;
__IO   uint8_t  TX_Enabled = 0;
struct pbuf *p;

struct ip_addr DestIPaddr;
#define LED1 GPIO_Pin_9
#define LED2 GPIO_Pin_11
#define LED3 GPIO_Pin_13
#define LED4 GPIO_Pin_15
#define PGA0 GPIO_Pin_3
#define PGA1 GPIO_Pin_2
volatile char do_blink;
/**************************************************************************************/

void RCC_Configuration(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}

/**************************************************************************************/

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* ADC Channel 10 -> PC0
     ADC Channel 12 -> PC2
     ADC Channel 13 -> PC3
	 */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 |GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	  /* Configure the GPIO_LED pin */
	  GPIO_InitStructure.GPIO_Pin = PGA0 | PGA1 | LED1 | LED2 | LED3 | LED4;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  GPIO_Init(GPIOE, &GPIO_InitStructure);


}

/**************************************************************************************/

void ADC_Configuration(void)
{
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	/* ADC Common Init */
	ADC_CommonInitStructure.ADC_Mode = ADC_TripleMode_RegSimult;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1; // DMA mode 1 enabled (2 / 3 half-words one by one - 1 then 2 then 3)
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE; // 1 Channel
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // Conversions Triggered
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_Init(ADC2, &ADC_InitStructure); // Mirror on ADC2
	ADC_Init(ADC3, &ADC_InitStructure); // Mirror on ADC3

	/* ADC1 regular channel 10 configuration */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_15Cycles); // PC1

	/* ADC2 regular channel 12 configuration */
	ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 1, ADC_SampleTime_15Cycles); // PC2

	/* ADC3 regular channel 13 configuration */
	ADC_RegularChannelConfig(ADC3, ADC_Channel_13, 1, ADC_SampleTime_15Cycles); // PC3
	/* Enable DMA request after last transfer (Multi-ADC mode)  */
	ADC_MultiModeDMARequestAfterLastTransferCmd(ENABLE);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);

	/* Enable ADC2 */
	ADC_Cmd(ADC2, ENABLE);

	/* Enable ADC3 */
	ADC_Cmd(ADC3, ENABLE);

}

/**************************************************************************************/


static void DMA_Configuration(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADCTripleConvertedValues[0];
	// DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)0x40012308; // CDR_ADDRESS; Packed ADC1, ADC2 add pack for addr 3 ??
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC_CDR_ADDRESS; // CDR_ADDRESS; Packed ADC1, ADC2 add pack for addr 3 ??

	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = BUFFERSIZE; // Count of 16-bit words
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;

	//DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;

	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);

	//DMA double buffer config
	DMA_DoubleBufferModeConfig(DMA2_Stream0,(uint32_t)&ADCTripleConvertedValuesShadow,DMA_Memory_0);
	DMA_DoubleBufferModeCmd(DMA2_Stream0,ENABLE);
	  /* Enable DMA Stream Half / Transfer Complete interrupt */
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC | DMA_IT_HT, ENABLE);

	/* DMA2_Stream0 enable */
	DMA_Cmd(DMA2_Stream0, ENABLE);
}

/**************************************************************************************/

void TIM2_Configuration(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	/* Time base configuration */
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Period = (84000000 / 200000) - 1; // 200 KHz, from 84 MHz TIM2CLK (ie APB1 = HCLK/4, TIM2CLK = HCLK/2)
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/* TIM2 TRGO selection */
	TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update); // ADC_ExternalTrigConv_T2_TRGO

	/* TIM2 enable counter */
	TIM_Cmd(TIM2, ENABLE);
}

/**************************************************************************************/

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the DMA Stream IRQ Channel */
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/**************************************************************************************/

void DMA2_Stream0_IRQHandler(void) // Called at 1 KHz for 200 KHz sample rate, LED Toggles at 500 Hz
{
	/* Test on DMA Stream Half Transfer interrupt */
	GPIO_ToggleBits(GPIOE,LED3);
	if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_HTIF0))
	{
		/* Clear DMA Stream Half Transfer interrupt pending bit */
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_HTIF0);
		if(DMA_GetCurrentMemoryTarget(DMA2_Stream0) == 0)
		{
			doADCTransfer = 1;

			GPIO_ResetBits(GPIOE, LED4);

		}
		else if(DMA_GetCurrentMemoryTarget(DMA2_Stream0) == 1)
		{
			GPIO_ResetBits(GPIOE, LED4);
		}

	}

	/* Test on DMA Stream Transfer Complete interrupt */
	if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0))
	{
		/* Clear DMA Stream Transfer Complete interrupt pending bit */
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);



	}
}
void Time_Update(void)
{
	static int c = 0;
	LocalTime += SYSTEMTICK_PERIOD_MS;
	if(c >= 100)
	{
		if(do_blink)
		{
		GPIO_ToggleBits(GPIOE, LED1);
		}
		c = 0;
	}
	c++;
}
/**************************************************************************************/

int main(void)
{
	unsigned int i =0;
	do_blink = 0;
	for(i = 0; i < BUFFERSIZE; i++)
	{
		ADCTripleConvertedValuesShadow[i]=i;
	}
	RCC_Configuration();
	RCC_GetClocksFreq(&RCC_Clocks);
	GPIO_Configuration();
	GPIO_SetBits(GPIOE,LED1);
	GPIO_SetBits(GPIOE,LED2);
	GPIO_SetBits(GPIOE,LED3);
	GPIO_SetBits(GPIOE,LED4);

	GPIO_SetBits(GPIOE,PGA0);
	GPIO_SetBits(GPIOE,PGA1);

	NVIC_Configuration();
	TIM2_Configuration();
	DMA_Configuration();
	ADC_Configuration();

	while (SysTick_Config(SystemCoreClock / 100) != 0) {
	} // One SysTick interrupt now equals 1us
	GPIO_ResetBits(GPIOE,LED4);


	/****************************/

	/* configure ethernet (GPIOs, clocks, MAC, DMA) */
	ETH_BSP_Config();



	TX_Enabled = 1;          // ISR can now TX data



	/****************************/

	/* Initilaize the LwIP stack */
	GPIO_SetBits(GPIOE,LED3);
	LwIP_Init();
	//Httpd init
	//httpd_init();
	/* create a UDP process control block */
	upcb = udp_new();
	/* configure destination IP address and port */
	IP4_ADDR(&DestIPaddr, DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3 );
	/* configure destination IP address and port */
	p = pbuf_alloc(PBUF_TRANSPORT, sizeof(ADCTripleConvertedValues), PBUF_RAM);


	GPIO_InitTypeDef GPIO_InitStructure;

	/* ADC Channel 10 -> PC0
     ADC Channel 12 -> PC2
     ADC Channel 13 -> PC3
	 */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 |GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	/* Start ADC1 Software Conversion */
	ADC_SoftwareStartConv(ADC1);

	//GPIO_ResetBits(GPIOE,LED4);
	do_blink = 1;
	uint32_t t = 0;
	while (1)
	{
		GPIO_ToggleBits(GPIOE,LED2);
		//Check if ADC buffer is ready to send

		if(doADCTransfer && p != NULL)
		{
			memcpy(p->payload,ADCTripleConvertedValuesShadow,sizeof(ADCTripleConvertedValues));
			udp_sendto(upcb, p, &DestIPaddr, 8889 );

			doADCTransfer = 0;
		}
		// check if any packet received
		if (ETH_CheckFrameReceived()) {
			// process received ethernet packet
			LwIP_Pkt_Handle();
		}
		// handle periodic timers for LwIP
		LwIP_Periodic_Handle(LocalTime);

	}
	// Don't need these if program never exits
	/* free pbuf */
	pbuf_free(p);
	/* free the UDP connection */
	udp_remove(upcb);
}
