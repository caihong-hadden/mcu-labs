#ifndef __ADC_H
#define __ADC_H	
#include "sys.h" 

#define USE_ADC						ADC2
#define USE_ADC_CLK					RCC_APB2Periph_ADC2
#define	ADC_CLK_DIV					ADC_Prescaler_Div4
//������ADC���õĺ궨��
#define	USE_ADC_GPIO_PORT		GPIOC
#define USE_ADC_GPIO_PIN		GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5
#define USE_ADC_GPIO_CLK		RCC_AHB1Periph_GPIOC
//������GPIO���õĺ궨��
#define USE_ADC_GPIO_SPEED	ADC_SampleTime_28Cycles
 							   
void Adc_Init(void); 				//ADCͨ����ʼ��
u16  Get_Adc(u8 ch); 				//���ĳ��ͨ��ֵ 
u16 Get_Adc_Average(u8 ch,u8 times);//�õ�ĳ��ͨ����������������ƽ��ֵ  

int Get_Adc_Med_Ave_Double(u8 ch1,u8 ch2,u8 times,u8 num);
#endif 















