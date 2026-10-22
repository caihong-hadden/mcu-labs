#include "telemetry.h"
#include <stdarg.h>
#include <string.h>

static UART_HandleTypeDef huart1;
static UART_HandleTypeDef huart3;

void Telemetry_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_USART3_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // 1. USART1: PA9(TX 复用推挽), PA10(RX 上拉输入)
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // 2. USART3: PB10(TX 复用推挽), PB11(RX 上拉输入)
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // 3. 配置 USART1 (115200 8N1)
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart1);

  // 4. 配置 USART3 (115200 8N1)
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart3);

  Telemetry_Printf("\r\n\r\n=======================================================\r\n");
  Telemetry_Printf(" [SYS] STM32F103 Super Mario Engine v2.0 Online!\r\n");
  Telemetry_Printf(" [SYS] Core Clock: 64MHz (HSI-PLL) | Flash Latency: 2 WS\r\n");
  Telemetry_Printf(" [SYS] Screen: 0.96 OLED (PB6=SDA, PB7=SCL) Fast I2C (~450kHz)\r\n");
  Telemetry_Printf(" [SYS] Keys: PA0(Jump), PB12(Left), PB13(Right), PB9(Action)\r\n");
  Telemetry_Printf(" [SYS] PS2 Gamepad: PA6(DAT), PA3(CMD), PA4(CS), PA5(CLK)\r\n");
  Telemetry_Printf(" [SYS] Dual Telemetry: USART1(PA9) & USART3(PB10) @ 115200 8N1\r\n");
  Telemetry_Printf("=======================================================\r\n\r\n");
}

void Telemetry_SendByte(uint8_t ch)
{
  // 双通道同步发送，内置超时防卡死
  uint32_t t1 = 5000;
  while (!(USART1->SR & USART_SR_TXE) && --t1);
  if (t1) USART1->DR = ch;

  uint32_t t3 = 5000;
  while (!(USART3->SR & USART_SR_TXE) && --t3);
  if (t3) USART3->DR = ch;
}

void Telemetry_SendStr(const char *str)
{
  while (*str)
  {
    Telemetry_SendByte((uint8_t)*str++);
  }
}

void Telemetry_Printf(const char *fmt, ...)
{
  char buf[160];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  Telemetry_SendStr(buf);
}

void Telemetry_LogKey(const char *key_name, uint8_t pressed)
{
  Telemetry_Printf("[KEY] %-6s -> %s\r\n", key_name, pressed ? "PRESSED  (LOW)" : "RELEASED (HIGH)");
}

void Telemetry_LogEvent(const char *event_name)
{
  Telemetry_Printf("[EVENT] >>> %s <<<\r\n", event_name);
}

void Telemetry_LogFrame(uint32_t fps, uint32_t frame_ms, uint32_t oled_ms, uint32_t logic_ms,
                        int16_t mario_x, int16_t mario_y, int16_t cam_x,
                        uint8_t kj, uint8_t kl, uint8_t kr, uint8_t ka,
                        const char *state_str, const char *ps2_str)
{
  Telemetry_Printf("[DIAG] FPS:%-2lu | Total:%2lums(OLED:%2lums, Logic:%2lums) | Mario:(%3d,%2d) Cam:%3d | PS2:%-8s | Keys:[J:%d L:%d R:%d A:%d] | State:%s\r\n",
                   fps, frame_ms, oled_ms, logic_ms, mario_x, mario_y, cam_x, ps2_str, kj, kl, kr, ka, state_str);
}
