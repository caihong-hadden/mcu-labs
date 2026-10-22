#ifndef __OLED_H
#define __OLED_H

#include "main.h"

#define OLED_WIDTH  128
#define OLED_HEIGHT 64

// 屏幕引脚：PB6 = SDA, PB7 = SCL (软件 I2C)
#define OLED_I2C_PORT GPIOB
#define OLED_SDA_PIN  GPIO_PIN_6
#define OLED_SCL_PIN  GPIO_PIN_7

extern uint8_t oled_addr;

uint8_t OLED_ScanAndInit(void);
void OLED_Init(void);
void OLED_Clear(void);
void OLED_UpdateScreen(void);

void OLED_WriteCmd(uint8_t cmd);
void OLED_WriteData(uint8_t data);

void OLED_DrawPixel(int16_t x, int16_t y, uint8_t color);
void OLED_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color);
void OLED_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);
void OLED_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);

void OLED_DrawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t flip_x);
void OLED_PrintChar(int16_t x, int16_t y, char ch);
void OLED_PrintStr(int16_t x, int16_t y, const char *str);
void OLED_PrintNum(int16_t x, int16_t y, int32_t num, uint8_t digits);

#endif /* __OLED_H */
