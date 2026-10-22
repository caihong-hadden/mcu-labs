#include <STC15F2K60S2.H>
#include "timer0.h"
#include "key.h"
#include <INTRINS.H>
//void Tmer0_Init()
//{

////		TMOD=0x01;//00000001
//	TMOD=&0xF0;//低四位清零，高四位保持不变；
//	TMOD=|0x01;//低四位置1，高四位还是保持不变；
//		TF0=0;
//		TR0=1;
//		TH0=64535/256;
//		TL0=64535%256;
//		ET0=1;
//		EA=1;
//		PT0=0;
//}
//void Timer0_Init(void)		//1毫秒@11.0592MHz
//{
//	AUXR |= 0x80;			//定时器时钟1T模式
//	TMOD &= 0xF0;			//设置定时器模式
//	TL0 = 0xCD;				//设置定时初始值
//	TH0 = 0xD4;				//设置定时初始值
//	TF0 = 0;				//清除TF0标志
//	TR0 = 1;				//定时器0开始计时
//	ET0=1;
//	EA=1;
//	PT0=0;
//}
unsigned char KeyNum,LEDMODE;
void main()
{
	P42=0;P27=1;P26=0;P25=0;
	P0=0xfE;
	Timer0_Init();
while(1)
{
		KeyNum=Key();
	  if(KeyNum){
		if(KeyNum==1) {
		LEDMODE++;
			if(LEDMODE>=2)LEDMODE=0;
		}
		
	
	}

}

}
void Timer0_Routine() interrupt 1
{
	 static unsigned int T0Count;
		T0Count++;
		if(T0Count>=500){
		T0Count=0;
		if(LEDMODE==0){
			P0=_crol_(P0,1);
			}
		if(LEDMODE==1){
			P0=_cror_(P0,1);
			}
		}
}