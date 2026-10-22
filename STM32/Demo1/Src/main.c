#include <stdint.h>
#include "stm32g4xx.h" // 必须替换为 G4 系列的头文件

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

int main(void)
{
    // 1. 开启 GPIOC 的时钟 (G4 系列的 GPIO 时钟在 AHB2ENR 寄存器)
    RCC->AHB2ENR |= (1 << 2); // 第2位对应 GPIOC

    // 2. 配置 PC13 为通用输出模式
    // 先将 PC13 对应的 MODER 位清零 (第26和27位)
    GPIOC->MODER &= ~(0x3 << (13 * 2));
    // 再将 PC13 对应的 MODER 位设置为 01 (通用输出模式)
    GPIOC->MODER |= (0x1 << (13 * 2));

    // 3. 设置 PC13 输出高电平 (ODR 寄存器用法与 F1 相同)
    GPIOC->ODR = 0X00002000;

    while(1){

    }
}
