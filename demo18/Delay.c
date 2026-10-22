#include <intrins.h>
void Delay(unsigned int xms)	//@11.0592MHz
{
	unsigned char data i, j;
while(xms--){
	_nop_();
	i = 11;
	j = 190;
	do
	{
		while (--j);
	} while (--i);
}
}