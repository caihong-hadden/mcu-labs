#include "ps2.h"
#include <string.h>

static uint8_t ps2_data[9] = {0};
static uint8_t ps2_mode = PSB_LOSE;
static uint16_t ps2_handkey = 0xFFFF; // 16位按键，1表示松开，0表示按下

static const uint8_t cmd_poll[9] = {0x01, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static void PS2_Delay_US(uint32_t us)
{
  for (volatile uint32_t i = 0; i < us * 8; i++);
}

static inline void PS2_CS_H(void)  { PS2_PORT->BSRR = PS2_CS_PIN; }
static inline void PS2_CS_L(void)  { PS2_PORT->BRR  = PS2_CS_PIN; }
static inline void PS2_SCK_H(void) { PS2_PORT->BSRR = PS2_SCK_PIN; }
static inline void PS2_SCK_L(void) { PS2_PORT->BRR  = PS2_SCK_PIN; }
static inline void PS2_DO_H(void)  { PS2_PORT->BSRR = PS2_DO_PIN; }
static inline void PS2_DO_L(void)  { PS2_PORT->BRR  = PS2_DO_PIN; }
static inline uint8_t PS2_DI(void) { return (PS2_PORT->IDR & PS2_DI_PIN) ? 1 : 0; }

static uint8_t PS2_Cmd(uint8_t cmd)
{
  uint8_t res = 0;
  for (uint8_t i = 0; i < 8; i++)
  {
    if (cmd & 0x01)
      PS2_DO_H();
    else
      PS2_DO_L();
    cmd >>= 1;

    PS2_Delay_US(8);
    PS2_SCK_L();
    PS2_Delay_US(8);
    if (PS2_DI())
    {
      res |= (1 << i);
    }
    PS2_SCK_H();
    PS2_Delay_US(8);
  }
  return res;
}

static void PS2_ShortPoll(void)
{
  PS2_CS_L();
  PS2_Delay_US(16);
  PS2_Cmd(0x01);
  PS2_Cmd(0x42);
  PS2_Cmd(0x00);
  PS2_Cmd(0x00);
  PS2_Cmd(0x00);
  PS2_CS_H();
  PS2_Delay_US(16);
}

static void PS2_EnterConfig(void)
{
  PS2_CS_L();
  PS2_Delay_US(16);
  PS2_Cmd(0x01);
  PS2_Cmd(0x43);
  PS2_Cmd(0x00);
  PS2_Cmd(0x01);
  PS2_Cmd(0x00);
  PS2_Cmd(0x00);
  PS2_Cmd(0x00);
  PS2_Cmd(0x00);
  PS2_Cmd(0x00);
  PS2_CS_H();
  PS2_Delay_US(16);
}

static void PS2_TurnOnAnalogMode(void)
{
  PS2_CS_L();
  PS2_Delay_US(16);
  PS2_Cmd(0x01);
  PS2_Cmd(0x44);
  PS2_Cmd(0x00);
  PS2_Cmd(0x01); // 0x01: 模拟模式开启 (红灯)
  PS2_Cmd(0x03); // 允许按 MODE 键切换红绿灯
  PS2_Cmd(0x00);
  PS2_Cmd(0x00);
  PS2_Cmd(0x00);
  PS2_Cmd(0x00);
  PS2_CS_H();
  PS2_Delay_US(16);
}

static void PS2_ExitConfig(void)
{
  PS2_CS_L();
  PS2_Delay_US(16);
  PS2_Cmd(0x01);
  PS2_Cmd(0x43);
  PS2_Cmd(0x00);
  PS2_Cmd(0x00);
  PS2_Cmd(0x5A);
  PS2_Cmd(0x5A);
  PS2_Cmd(0x5A);
  PS2_Cmd(0x5A);
  PS2_Cmd(0x5A);
  PS2_CS_H();
  PS2_Delay_US(16);
}

void PS2_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // 1. DO(PA3), CS(PA4), SCK(PA5) -> 推挽输出强驱动
  GPIO_InitStruct.Pin = PS2_DO_PIN | PS2_CS_PIN | PS2_SCK_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(PS2_PORT, &GPIO_InitStruct);

  // 2. DI(PA6) -> 输入上拉
  GPIO_InitStruct.Pin = PS2_DI_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(PS2_PORT, &GPIO_InitStruct);

  PS2_SCK_H();
  PS2_CS_H();
  PS2_DO_H();
  HAL_Delay(30);

  // 3. 握手与初始化序列
  PS2_ShortPoll();
  PS2_ShortPoll();
  PS2_ShortPoll();
  PS2_EnterConfig();
  PS2_TurnOnAnalogMode();
  PS2_ExitConfig();

  HAL_Delay(10);
  PS2_ReadData();
}

uint8_t PS2_ReadData(void)
{
  PS2_CS_L();
  PS2_Delay_US(16);

  for (uint8_t i = 0; i < 9; i++)
  {
    if (i < 2)
      ps2_data[i] = PS2_Cmd(cmd_poll[i]);
    else
      ps2_data[i] = PS2_Cmd(0x00);
  }

  PS2_CS_H();
  PS2_Delay_US(16);

  // 判断接收模式
  if (ps2_data[1] == 0x73)
  {
    ps2_mode = PSB_REDLIGHT_MODE;
  }
  else if (ps2_data[1] == 0x41)
  {
    ps2_mode = PSB_GREENLIGHT_MODE;
  }
  else
  {
    ps2_mode = PSB_LOSE;
  }

  if (ps2_mode != PSB_LOSE)
  {
    ps2_handkey = (uint16_t)(ps2_data[4] << 8) | ps2_data[3];
  }
  else
  {
    ps2_handkey = 0xFFFF; // 离线状态设为全部未按下
  }

  return ps2_mode;
}

uint8_t PS2_GetKeyState(uint8_t key_id)
{
  if (ps2_mode == PSB_LOSE || key_id < PSB_SELECT || key_id > PSB_SQUARE)
    return 0;
  // 低电平有效：0 代表按下，返回 1
  return ((ps2_handkey & (1 << (key_id - 1))) == 0) ? 1 : 0;
}

uint8_t PS2_GetAnalogData(uint8_t stick_id)
{
  if (ps2_mode == PSB_LOSE || stick_id < 5 || stick_id > 8)
    return 128; // 中位默认值
  return ps2_data[stick_id];
}

uint8_t PS2_GetMode(void)
{
  return ps2_mode;
}

uint16_t PS2_GetRawKeys(void)
{
  return ps2_handkey;
}
