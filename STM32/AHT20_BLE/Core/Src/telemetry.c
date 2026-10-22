#include "telemetry.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef huart1;
static UART_HandleTypeDef huart3;

#define BLE_RX_BUF_SIZE 128
static volatile char ble_rx_buf[BLE_RX_BUF_SIZE];
static volatile uint8_t ble_rx_head = 0;
static volatile uint8_t ble_rx_tail = 0;

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

  // 3. 配置 USART1 (115200 8N1) - 电脑 USB-TTL
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart1);

  // 4. 配置 USART3 (115200 8N1) - PB-03F 蓝牙透传
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart3);

  // 开启 USART3 接收中断
  __HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
  HAL_NVIC_SetPriority(USART3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USART3_IRQn);
}

void USART3_IRQHandler(void)
{
  if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE))
  {
    char c = (char)(huart3.Instance->DR & 0xFF);
    uint8_t next_head = (ble_rx_head + 1) % BLE_RX_BUF_SIZE;
    if (next_head != ble_rx_tail)
    {
      ble_rx_buf[ble_rx_head] = c;
      ble_rx_head = next_head;
    }
  }
}

/* 双通道并发广播 (同时向电脑终端和蓝牙手机发送) */
void Telemetry_Printf(const char *fmt, ...)
{
  char buf[256];
  va_list args;
  va_start(args, fmt);
  int len = vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if (len <= 0) return;

  // 1. 发送到 USART1 (PC 电脑 USB-TTL)
  HAL_UART_Transmit(&huart1, (uint8_t *)buf, (uint16_t)len, 50);

  // 2. 发送到 USART3 (PB-03F 蓝牙模块透传至手机/无线上位机)
  HAL_UART_Transmit(&huart3, (uint8_t *)buf, (uint16_t)len, 50);
}

/* 读取蓝牙端发来的单行文本指令 (非阻塞) */
uint8_t Telemetry_ReadBLERx(char *out_buf, uint16_t max_len)
{
  static char line_buf[64];
  static uint8_t line_idx = 0;

  while (ble_rx_tail != ble_rx_head)
  {
    char c = ble_rx_buf[ble_rx_tail];
    ble_rx_tail = (ble_rx_tail + 1) % BLE_RX_BUF_SIZE;

    if (c == '\r' || c == '\n')
    {
      if (line_idx > 0)
      {
        line_buf[line_idx] = '\0';
        strncpy(out_buf, line_buf, max_len - 1);
        out_buf[max_len - 1] = '\0';
        line_idx = 0;
        return 1; // 成功解析出一整行
      }
    }
    else
    {
      if (line_idx < sizeof(line_buf) - 1)
      {
        line_buf[line_idx++] = c;
      }
    }
  }
  return 0;
}
