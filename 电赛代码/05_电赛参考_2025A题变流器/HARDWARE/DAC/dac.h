#ifndef __DAC_H
#define __DAC_H	 
#include "sys.h"

#define DAC_CLK							RCC_APB1Periph_DAC
#define DAC_GPIO_PORT				GPIOA
#define DAC_GPIO_PIN				GPIO_Pin_4
#define DAC_GPIO_CLK				RCC_AHB1Periph_GPIOA
#define DAC_CHANNEL					DAC_Channel_1

void Dac1_Init(void);		//DAC通道1初始化	 	 

#endif

















