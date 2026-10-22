#include <STC15F2K60S2.H>

void Timer0_Init(void)		//1毫秒@11.0592MHz
{
	AUXR |= 0x80;			//定时器时钟1T模式
	TMOD &= 0xF0;			//设置定时器模式
	TL0 = 0xCD;				//设置定时初始值
	TH0 = 0xD4;				//设置定时初始值
	TF0 = 0;				//清除TF0标志
	TR0 = 1;				//定时器0开始计时
	ET0=1;
	EA=1;
	PT0=0;
}
//void Timer0_Routine() interrupt 1
//{
//	 static unsigned int T0Count;
//		T0Count++;
//		if(T0Count>=1000){
//		T0Count=0;            1s模板
//		}
//}