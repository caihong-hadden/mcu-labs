#include <STC15F2K60S2.H>

/**
 * @brief 串口1初始化函数
 *        配置为: 8位数据位, 无校验位, 1位停止位 (8-N-1)
 *        波特率: 115200 bps @ 12.000MHz (1T 模式)
 */
void UART_Init(void)
{
    SCON = 0x50;  // 工作模式1(8位UART), 允许串口接收 (REN=1)
    AUXR |= 0x40; // 定时器1时钟设为 1T 模式 
    AUXR &= 0xFE; // 串口1波特率发生器选用定时器1 
    
    TMOD &= 0x0F; // 清空定时器1的配置位
    TMOD |= 0x00; // 定时器1配置为16位自动重装载模式
    
    // 修正波特率初值计算: 65536 - 26 = 65510，十六进制为 0xFFE6
    TL1 = 0xE6;   // 【修正】原来是 0xE8
    TH1 = 0xFF;   
    
    ET1 = 0;      // 禁止定时器1中断 
    TR1 = 1;      // 启动定时器1计数
    
    EA  = 1;      // 开启CPU总中断
    ES  = 0;      // 【致命修正】关闭串口1中断，防止 while(TI==0) 死锁
}

void UART_SendByte(unsigned char Byte)
{
    SBUF = Byte;    
    while(TI == 0); // 此时关闭了串口中断，这里就能安全地死等发送完成了
    TI = 0;         
}

void UART_SendString(char *str)
{
    while(*str != '\0') 
    {
        UART_SendByte(*str);
        str++;
    }
}