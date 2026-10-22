#include "adc.h"
#include "delay.h"		 
#include "global.h"

//��ʼ��ADC															   
void  Adc_Init(void)
{    
  GPIO_InitTypeDef  		GPIO_InitStructure;
  ADC_CommonInitTypeDef 	ADC_CommonInitStructure;
  ADC_InitTypeDef       	ADC_InitStructure;
	
  RCC_AHB1PeriphClockCmd(USE_ADC_GPIO_CLK, ENABLE);//ʹ��GPIOAʱ��
  RCC_APB2PeriphClockCmd(USE_ADC_CLK, ENABLE); //ʹ��ADC1ʱ��

  //�ȳ�ʼ��ADC1ͨ��5 IO��
  GPIO_InitStructure.GPIO_Pin  = USE_ADC_GPIO_PIN;//PA5 ͨ��5
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//ģ������
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//����������
  GPIO_Init(USE_ADC_GPIO_PORT, &GPIO_InitStructure);//��ʼ��  

  //�ȳ�ʼ��ADC1ͨ��5 IO��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;//PA5 ͨ��5
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//ģ������
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��  
 
  RCC_APB2PeriphResetCmd(USE_ADC_CLK,ENABLE);	  //ADC1��λ
  RCC_APB2PeriphResetCmd(USE_ADC_CLK,DISABLE);	//��λ����	 
 
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//����ģʽ
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//���������׶�֮����ӳ�5��ʱ��
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMAʧ��
  ADC_CommonInitStructure.ADC_Prescaler = ADC_CLK_DIV;//Ԥ��Ƶ4��Ƶ��ADCCLK=PCLK2/4=84/4=21Mhz,ADCʱ����ò�Ҫ����36Mhz 
  ADC_CommonInit(&ADC_CommonInitStructure);//��ʼ��
	
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12λģʽ
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;//��ɨ��ģʽ	
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//�ر�����ת��
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//��ֹ������⣬ʹ����������
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//�Ҷ���	
  ADC_InitStructure.ADC_NbrOfConversion = 1;//1��ת���ڹ��������� Ҳ����ֻת����������1 
  ADC_Init(USE_ADC, &ADC_InitStructure);//ADC��ʼ��
	
  ADC_Cmd(USE_ADC, ENABLE);//����ADת����	

}

u16 Get_Adc(u8 ch)   
{
	  	//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(USE_ADC, ch, 1, USE_ADC_GPIO_SPEED );	//ADC1,ADCͨ��,480������,��߲���ʱ�������߾�ȷ��			    
  
	ADC_SoftwareStartConv(USE_ADC);		//ʹ��ָ����ADC1������ת����������	
	 
	while(!ADC_GetFlagStatus(USE_ADC, ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(USE_ADC);	//�������һ��ADC1�������ת�����
}

u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);
		delay_us(1);
	}
	return temp_val/times;
} 

u16 Get_Adc_Med_Ave(u8 ch,u8 times,u8 num)
{
	int temp_val[100];
	u32 result=0,adcx;
	u8 t,i,j=0;
	for(t=1;t<=times;t++)
	{
			temp_val[t]=Get_Adc(ch);
			delay_us(1);
	}
	qusort(temp_val,1,times);
	i=(int)times/2-num/2;
	while(j<num)
	{
			result = result + temp_val[i+j];
			j++;
	}
	adcx = result/num;
	return adcx;
}
	 











