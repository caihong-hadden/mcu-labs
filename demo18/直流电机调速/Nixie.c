#include <STC15F2K60S2.H>
unsigned char NumberTable [] ={0xc0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xff};
//unsigned char Numberloc [] ={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

void Nixie(unsigned char loc,unsigned char Num){
				P27=1;P26=1;P42=1;

//			switch(location){
//				case 1:P27=1;P26=1;P25=0;P0=0x01;break;
//				case 2:P27=1;P26=1;P25=0;P0=0x02;break;
//				case 3:P27=1;P26=1;P25=0;P0=0x04;break;
//				case 4:P27=1;P26=1;P25=0;P0=0x08;break;
//				case 5:P27=1;P26=1;P25=0;P0=0x10;break;
//				case 6:P27=1;P26=1;P25=0;P0=0x20;break;
//				case 7:P27=1;P26=1;P25=0;P0=0x40;break;
//				case 8:P27=1;P26=1;P25=0;P0=0x80;break;
//			
//			}
//			switch(Number){
//			  case 0:P27=1;P26=1;P25=1;P0=0xc0;break;
//				case 1:P27=1;P26=1;P25=1;P0=0xF9;break;
//				case 2:P27=1;P26=1;P25=1;P0=0xA4;break;
//				case 3:P27=1;P26=1;P25=1;P0=0xB0;break;
//				case 4:P27=1;P26=1;P25=1;P0=0x99;break;
//				case 5:P27=1;P26=1;P25=1;P0=0x92;break;
//				case 6:P27=1;P26=1;P25=1;P0=0x82;break;
//				case 7:P27=1;P26=1;P25=1;P0=0xF8;break;
//				case 8:P27=1;P26=1;P25=1;P0=0x80;break;
//				case 9:P27=1;P26=1;P25=1;P0=0x90;break;
//			
//			}
			P25=1;P0=0xff;
			P25=0;P0=(1<<loc-1);//位移寻址
			P25=1;
			P0=NumberTable[Num];

}