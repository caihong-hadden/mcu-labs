#include <STC15F2K60S2.H>
void BUZZER(unsigned char buzzer){
	if(buzzer==1){
P27=1;P26=0;P25=1;
P06=1;
P27=0;P26=0;P25=0;
	}
	if(buzzer==0){
P27=1;P26=0;P25=1;
P06=0;
P27=0;P26=0;P25=0;
}
	}