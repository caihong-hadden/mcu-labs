#include "oled.h"
#include <string.h>

static uint8_t oled_buffer[1024];
uint8_t oled_addr = 0x78; // 默认 0x78 (0x3C<<1) 或 0x7A (0x3D<<1)

/* ---------------- 健壮高极速 I2C 物理层时序 (纯寄存器 0 开销) ---------------- */
static inline void I2C_Delay(void)
{
  for (volatile int i = 0; i < 2; i++); // 64MHz 下约 150ns，实现稳健 400~500kHz Fast Mode I2C
}

static inline void I2C_SCL_H(void) { OLED_I2C_PORT->BSRR = OLED_SCL_PIN; }
static inline void I2C_SCL_L(void) { OLED_I2C_PORT->BRR  = OLED_SCL_PIN; }
static inline void I2C_SDA_H(void) { OLED_I2C_PORT->BSRR = OLED_SDA_PIN; }
static inline void I2C_SDA_L(void) { OLED_I2C_PORT->BRR  = OLED_SDA_PIN; }
static inline uint8_t I2C_SDA_READ(void) { return (OLED_I2C_PORT->IDR & OLED_SDA_PIN) ? 1 : 0; }

static void I2C_Start(void)
{
  I2C_SDA_H();
  I2C_SCL_H();
  I2C_Delay();
  I2C_SDA_L();
  I2C_Delay();
  I2C_SCL_L();
  I2C_Delay();
}

static void I2C_Stop(void)
{
  I2C_SCL_L();
  I2C_SDA_L();
  I2C_Delay();
  I2C_SCL_H();
  I2C_Delay();
  I2C_SDA_H();
  I2C_Delay();
}

static uint8_t I2C_WriteByte(uint8_t byte)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (byte & 0x80)
      I2C_SDA_H();
    else
      I2C_SDA_L();
    I2C_Delay();
    I2C_SCL_H();
    I2C_Delay();
    I2C_SCL_L();
    I2C_Delay();
    byte <<= 1;
  }

  // 接收从机 ACK (开漏模式下直接释放 SDA 置高，从机拉低产生 ACK，无需频繁重配模式！)
  I2C_SDA_H();
  I2C_Delay();
  I2C_SCL_H();
  I2C_Delay();
  uint8_t ack = I2C_SDA_READ(); // 0 = 收到从机响应 (ACK), 1 = 无应答 (NACK)
  I2C_SCL_L();
  I2C_Delay();
  return ack;
}

static uint8_t I2C_ProbeAddress(uint8_t addr)
{
  I2C_Start();
  uint8_t ack = I2C_WriteByte(addr);
  I2C_Stop();
  return (ack == 0);
}

void OLED_WriteCmd(uint8_t cmd)
{
  I2C_Start();
  I2C_WriteByte(oled_addr);
  I2C_WriteByte(0x00); // 0x00: 指令流
  I2C_WriteByte(cmd);
  I2C_Stop();
}

void OLED_WriteData(uint8_t data)
{
  I2C_Start();
  I2C_WriteByte(oled_addr);
  I2C_WriteByte(0x40); // 0x40: 数据流
  I2C_WriteByte(data);
  I2C_Stop();
}

static const uint8_t Font5x7[][5] = {
  {0x00, 0x00, 0x00, 0x00, 0x00}, // 32 ' '
  {0x00, 0x00, 0x5F, 0x00, 0x00}, // 33 '!'
  {0x00, 0x07, 0x00, 0x07, 0x00}, // 34 '"'
  {0x14, 0x7F, 0x14, 0x7F, 0x14}, // 35 '#'
  {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // 36 '$'
  {0x23, 0x13, 0x08, 0x64, 0x62}, // 37 '%'
  {0x36, 0x49, 0x55, 0x22, 0x50}, // 38 '&'
  {0x00, 0x05, 0x03, 0x00, 0x00}, // 39 '''
  {0x00, 0x1C, 0x22, 0x41, 0x00}, // 40 '('
  {0x00, 0x41, 0x22, 0x1C, 0x00}, // 41 ')'
  {0x14, 0x08, 0x3E, 0x08, 0x14}, // 42 '*'
  {0x08, 0x08, 0x3E, 0x08, 0x08}, // 43 '+'
  {0x00, 0x50, 0x30, 0x00, 0x00}, // 44 ','
  {0x08, 0x08, 0x08, 0x08, 0x08}, // 45 '-'
  {0x00, 0x60, 0x60, 0x00, 0x00}, // 46 '.'
  {0x20, 0x10, 0x08, 0x04, 0x02}, // 47 '/'
  {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 48 '0'
  {0x00, 0x42, 0x7F, 0x40, 0x00}, // 49 '1'
  {0x42, 0x61, 0x51, 0x49, 0x46}, // 50 '2'
  {0x21, 0x41, 0x45, 0x4B, 0x31}, // 51 '3'
  {0x18, 0x14, 0x12, 0x7F, 0x10}, // 52 '4'
  {0x27, 0x45, 0x45, 0x45, 0x39}, // 53 '5'
  {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 54 '6'
  {0x01, 0x71, 0x09, 0x05, 0x03}, // 55 '7'
  {0x36, 0x49, 0x49, 0x49, 0x36}, // 56 '8'
  {0x06, 0x49, 0x49, 0x29, 0x1E}, // 57 '9'
  {0x00, 0x36, 0x36, 0x00, 0x00}, // 58 ':'
  {0x00, 0x56, 0x36, 0x00, 0x00}, // 59 ';'
  {0x08, 0x14, 0x22, 0x41, 0x00}, // 60 '<'
  {0x14, 0x14, 0x14, 0x14, 0x14}, // 61 '='
  {0x00, 0x41, 0x22, 0x14, 0x08}, // 62 '>'
  {0x02, 0x01, 0x51, 0x09, 0x06}, // 63 '?'
  {0x32, 0x49, 0x79, 0x41, 0x3E}, // 64 '@'
  {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 65 'A'
  {0x7F, 0x49, 0x49, 0x49, 0x36}, // 66 'B'
  {0x3E, 0x41, 0x41, 0x41, 0x22}, // 67 'C'
  {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 68 'D'
  {0x7F, 0x49, 0x49, 0x49, 0x41}, // 69 'E'
  {0x7F, 0x09, 0x09, 0x09, 0x01}, // 70 'F'
  {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 71 'G'
  {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 72 'H'
  {0x00, 0x41, 0x7F, 0x41, 0x00}, // 73 'I'
  {0x20, 0x40, 0x41, 0x3F, 0x01}, // 74 'J'
  {0x7F, 0x08, 0x14, 0x22, 0x41}, // 75 'K'
  {0x7F, 0x40, 0x40, 0x40, 0x40}, // 76 'L'
  {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 77 'M'
  {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 78 'N'
  {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 79 'O'
  {0x7F, 0x09, 0x09, 0x09, 0x06}, // 80 'P'
  {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 81 'Q'
  {0x7F, 0x09, 0x19, 0x29, 0x46}, // 82 'R'
  {0x46, 0x49, 0x49, 0x49, 0x31}, // 83 'S'
  {0x01, 0x01, 0x7F, 0x01, 0x01}, // 84 'T'
  {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 85 'U'
  {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 86 'V'
  {0x3F, 0x40, 0x38, 0x40, 0x3F}, // 87 'W'
  {0x63, 0x14, 0x08, 0x14, 0x63}, // 88 'X'
  {0x07, 0x08, 0x70, 0x08, 0x07}, // 89 'Y'
  {0x61, 0x51, 0x49, 0x45, 0x43}, // 90 'Z'
  {0x00, 0x7F, 0x41, 0x41, 0x00}, // 91 '['
  {0x02, 0x04, 0x08, 0x10, 0x20}, // 92 '\'
  {0x00, 0x41, 0x41, 0x7F, 0x00}, // 93 ']'
  {0x04, 0x02, 0x01, 0x02, 0x04}, // 94 '^'
  {0x40, 0x40, 0x40, 0x40, 0x40}, // 95 '_'
  {0x00, 0x01, 0x02, 0x04, 0x00}, // 96 '`'
  {0x20, 0x54, 0x54, 0x54, 0x78}, // 97 'a'
  {0x7F, 0x48, 0x44, 0x44, 0x38}, // 98 'b'
  {0x38, 0x44, 0x44, 0x44, 0x20}, // 99 'c'
  {0x38, 0x44, 0x44, 0x48, 0x7F}, // 100 'd'
  {0x38, 0x54, 0x54, 0x54, 0x18}, // 101 'e'
  {0x08, 0x7E, 0x09, 0x01, 0x02}, // 102 'f'
  {0x0C, 0x52, 0x52, 0x52, 0x3E}, // 103 'g'
  {0x7F, 0x08, 0x04, 0x04, 0x78}, // 104 'h'
  {0x00, 0x44, 0x7D, 0x40, 0x00}, // 105 'i'
  {0x20, 0x40, 0x44, 0x3D, 0x00}, // 106 'j'
  {0x7F, 0x10, 0x28, 0x44, 0x00}, // 107 'k'
  {0x00, 0x41, 0x7F, 0x40, 0x00}, // 108 'l'
  {0x7C, 0x04, 0x18, 0x04, 0x78}, // 109 'm'
  {0x7C, 0x08, 0x04, 0x04, 0x78}, // 110 'n'
  {0x38, 0x44, 0x44, 0x44, 0x38}, // 111 'o'
  {0x7C, 0x14, 0x14, 0x14, 0x08}, // 112 'p'
  {0x08, 0x14, 0x14, 0x18, 0x7C}, // 113 'q'
  {0x7C, 0x08, 0x04, 0x04, 0x08}, // 114 'r'
  {0x48, 0x54, 0x54, 0x54, 0x20}, // 115 's'
  {0x04, 0x3F, 0x44, 0x40, 0x20}, // 116 't'
  {0x3C, 0x40, 0x40, 0x20, 0x7C}, // 117 'u'
  {0x1C, 0x20, 0x40, 0x20, 0x1C}, // 118 'v'
  {0x3C, 0x40, 0x30, 0x40, 0x3C}, // 119 'w'
  {0x44, 0x28, 0x10, 0x28, 0x44}, // 120 'x'
  {0x0C, 0x50, 0x50, 0x50, 0x3C}, // 121 'y'
  {0x44, 0x64, 0x54, 0x4C, 0x44}  // 122 'z'
};

uint8_t OLED_ScanAndInit(void)
{
  __HAL_RCC_GPIOB_CLK_ENABLE();

  // 1. SCL 配置为强推挽输出 (Master 纯净时钟输出，陡峭上升沿)
  //    SDA 配置为开漏输出 (I2C 标准开漏模式，支持双向应答与高速传输)
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = OLED_SCL_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(OLED_I2C_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = OLED_SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(OLED_I2C_PORT, &GPIO_InitStruct);

  I2C_SDA_H();
  I2C_SCL_H();
  HAL_Delay(100); // 等待 OLED 内部电容与上电复位完成

  // 2. 依次探测 0x78 (0x3C) 和 0x7A (0x3D)
  uint8_t found = 0;
  if (I2C_ProbeAddress(0x78))
  {
    oled_addr = 0x78;
    found = 1;
  }
  else if (I2C_ProbeAddress(0x7A))
  {
    oled_addr = 0x7A;
    found = 2;
  }
  else
  {
    // 盲扫全局 I2C
    for (uint16_t a = 2; a < 255; a += 2)
    {
      if (I2C_ProbeAddress((uint8_t)a))
      {
        oled_addr = (uint8_t)a;
        found = 3;
        break;
      }
    }
  }

  // 3. 发送双重高兼容初始化序列 (SSD1306 + SH1106 均可完美点亮)
  OLED_WriteCmd(0xAE); // 关闭屏幕
  OLED_WriteCmd(0x00); // 列低地址
  OLED_WriteCmd(0x10); // 列高地址
  OLED_WriteCmd(0x40); // 起始行 0
  OLED_WriteCmd(0xB0); // 页地址 0

  OLED_WriteCmd(0x81); // 对比度控制
  OLED_WriteCmd(0xFF); // 调至最高亮度 (255)

  OLED_WriteCmd(0xA1); // 段重映射 (正常左右方向)
  OLED_WriteCmd(0xC8); // COM 输出扫描反向
  OLED_WriteCmd(0xA6); // 正常显示 (未反相)
  OLED_WriteCmd(0xA8); // 多路复用比
  OLED_WriteCmd(0x3F); // 64行
  OLED_WriteCmd(0xD3); // 显示偏移 0
  OLED_WriteCmd(0x00);
  OLED_WriteCmd(0xD5); // 显示时钟分频
  OLED_WriteCmd(0x80);
  OLED_WriteCmd(0xD9); // 预充电周期
  OLED_WriteCmd(0xF1);
  OLED_WriteCmd(0xDA); // COM 引脚硬件配置
  OLED_WriteCmd(0x12);
  OLED_WriteCmd(0xDB); // VCOMH
  OLED_WriteCmd(0x40);

  // 【同时开启两款芯片内部升压泵】：
  // SSD1306 开启电荷泵: 0x8D, 0x14
  OLED_WriteCmd(0x8D);
  OLED_WriteCmd(0x14);

  // SH1106 开启内部 DC-DC: 0xAD, 0x8B
  OLED_WriteCmd(0xAD);
  OLED_WriteCmd(0x8B);

  OLED_WriteCmd(0xAF); // 开启屏幕显示 (Display ON)

  OLED_Clear();
  OLED_UpdateScreen();

  return found;
}

void OLED_Init(void)
{
  OLED_ScanAndInit();
}

void OLED_Clear(void)
{
  memset(oled_buffer, 0, sizeof(oled_buffer));
}

void OLED_UpdateScreen(void)
{
  for (uint8_t page = 0; page < 8; page++)
  {
    OLED_WriteCmd(0xB0 + page);
    OLED_WriteCmd(0x00); // 低列
    OLED_WriteCmd(0x10); // 高列

    I2C_Start();
    I2C_WriteByte(oled_addr);
    I2C_WriteByte(0x40); // 数据流
    for (uint8_t col = 0; col < 128; col++)
    {
      I2C_WriteByte(oled_buffer[page * 128 + col]);
    }
    I2C_Stop();
  }
}

void OLED_DrawPixel(int16_t x, int16_t y, uint8_t color)
{
  if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT)
    return;

  if (color)
    oled_buffer[x + (y / 8) * OLED_WIDTH] |= (1 << (y % 8));
  else
    oled_buffer[x + (y / 8) * OLED_WIDTH] &= ~(1 << (y % 8));
}

void OLED_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
  int16_t dx = (x1 >= x0) ? (x1 - x0) : (x0 - x1);
  int16_t dy = (y1 >= y0) ? (y1 - y0) : (y0 - y1);
  int16_t sx = (x0 < x1) ? 1 : -1;
  int16_t sy = (y0 < y1) ? 1 : -1;
  int16_t err = dx - dy;

  while (1)
  {
    OLED_DrawPixel(x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    int16_t e2 = 2 * err;
    if (e2 > -dy) { err -= dy; x0 += sx; }
    if (e2 < dx)  { err += dx; y0 += sy; }
  }
}

void OLED_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color)
{
  OLED_DrawLine(x, y, x + w - 1, y, color);
  OLED_DrawLine(x, y + h - 1, x + w - 1, y + h - 1, color);
  OLED_DrawLine(x, y, x, y + h - 1, color);
  OLED_DrawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
}

void OLED_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color)
{
  for (int16_t i = 0; i < w; i++)
  {
    for (int16_t j = 0; j < h; j++)
    {
      OLED_DrawPixel(x + i, y + j, color);
    }
  }
}

void OLED_DrawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t flip_x)
{
  uint8_t byte_width = (w + 7) / 8;
  for (uint8_t r = 0; r < h; r++)
  {
    for (uint8_t c = 0; c < w; c++)
    {
      uint8_t col_idx = flip_x ? (w - 1 - c) : c;
      uint8_t byte_val = bitmap[r * byte_width + (col_idx / 8)];
      if (byte_val & (0x80 >> (col_idx % 8)))
      {
        OLED_DrawPixel(x + c, y + r, 1);
      }
    }
  }
}

void OLED_PrintChar(int16_t x, int16_t y, char ch)
{
  if (ch < 32 || ch > 122) ch = '?';
  const uint8_t *char_data = Font5x7[ch - 32];
  for (uint8_t c = 0; c < 5; c++)
  {
    uint8_t line = char_data[c];
    for (uint8_t r = 0; r < 7; r++)
    {
      if (line & (1 << r))
      {
        OLED_DrawPixel(x + c, y + r, 1);
      }
    }
  }
}

void OLED_PrintStr(int16_t x, int16_t y, const char *str)
{
  while (*str)
  {
    OLED_PrintChar(x, y, *str);
    x += 6;
    str++;
  }
}

void OLED_PrintNum(int16_t x, int16_t y, int32_t num, uint8_t digits)
{
  char buf[12];
  if (num < 0) num = 0;
  for (int8_t i = digits - 1; i >= 0; i--)
  {
    buf[i] = '0' + (num % 10);
    num /= 10;
  }
  buf[digits] = '\0';
  OLED_PrintStr(x, y, buf);
}
