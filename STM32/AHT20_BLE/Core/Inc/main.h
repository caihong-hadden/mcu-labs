#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

// 板载状态指示灯定义
#define LED_PORT          GPIOB
#define LED_READY_PIN     GPIO_PIN_0
#define LED_BEAT_PIN      GPIO_PIN_1

// 蜂鸣器引脚 (PA7)
#define BUZZER_PORT       GPIOA
#define BUZZER_PIN        GPIO_PIN_7

// AHT20 温湿度传感器 I2C 引脚 (PB6 = SCL, PB7 = SDA)
#define AHT20_I2C_PORT    GPIOB
#define AHT20_SCL_PIN     GPIO_PIN_6
#define AHT20_SDA_PIN     GPIO_PIN_7

// PB-03F 蓝牙串口 (USART3 PB10=TX, PB11=RX)
// 调试串口 (USART1 PA9=TX, PA10=RX)

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
