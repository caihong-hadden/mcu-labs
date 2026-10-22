#include <STC15F2K60S2.H>
#include "Delay.h"

unsigned char MatrixKey(){

unsigned char KeyNum=0;
	P3=0xff;
	P31=0;
	if(P44==0){Delay(20);while(P44==0);Delay(20);KeyNum=6;}
	if(P42==0){Delay(20);while(P42==0);Delay(20);KeyNum=10;}
	if(P35==0){Delay(20);while(P35==0);Delay(20);KeyNum=14;}
	if(P34==0){Delay(20);while(P34==0);Delay(20);KeyNum=18;}
	
	P3=0xff;
	P30=0;
	if(P44==0){Delay(20);while(P44==0);Delay(20);KeyNum=7;}
	if(P42==0){Delay(20);while(P42==0);Delay(20);KeyNum=11;}
	if(P35==0){Delay(20);while(P35==0);Delay(20);KeyNum=15;}
	if(P34==0){Delay(20);while(P34==0);Delay(20);KeyNum=19;}
	
	P3=0xff;
	P32=0;
	if(P44==0){Delay(20);while(P44==0);Delay(20);KeyNum=5;}
	if(P42==0){Delay(20);while(P42==0);Delay(20);KeyNum=9;}
	if(P35==0){Delay(20);while(P35==0);Delay(20);KeyNum=13;}
	if(P34==0){Delay(20);while(P34==0);Delay(20);KeyNum=17;}
	
	P3=0xff;
	P33=0;
	if(P44==0){Delay(20);while(P44==0);Delay(20);KeyNum=4;}
	if(P42==0){Delay(20);while(P42==0);Delay(20);KeyNum=8;}
	if(P35==0){Delay(20);while(P35==0);Delay(20);KeyNum=12;}
	if(P34==0){Delay(20);while(P34==0);Delay(20);KeyNum=16;}
	return KeyNum;

}