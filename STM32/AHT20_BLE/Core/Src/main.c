#include "main.h"
#include "aht20.h"
#include "telemetry.h"
#include <stdio.h>
#include <string.h>

void SystemClock_Config(void);

// 蜂鸣器发声辅助
static void Buzzer_Tone(uint16_t freq, uint16_t ms)
{
  if (freq == 0 || ms == 0) return;
  uint32_t half_period_us = 500000 / freq;
  uint32_t cycles = ((uint32_t)freq * ms) / 1000;
  for (uint32_t i = 0; i < cycles; i++)
  {
    BUZZER_PORT->BSRR = BUZZER_PIN;
    for (volatile int d = 0; d < 64 * half_period_us / 10; d++);
    BUZZER_PORT->BRR  = BUZZER_PIN;
    for (volatile int d = 0; d < 64 * half_period_us / 10; d++);
  }
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  // 1. 初始化板载 LED (PB0, PB1) 与蜂鸣器 (PA7)
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // LED (PB0: 就绪灯, PB1: 心跳灯)
  GPIO_InitStruct.Pin = LED_READY_PIN | LED_BEAT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(LED_PORT, LED_READY_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_PORT, LED_BEAT_PIN, GPIO_PIN_RESET);

  // 蜂鸣器 (PA7)
  GPIO_InitStruct.Pin = BUZZER_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(BUZZER_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);

  // 2. 初始化双通道串口 (USART1 PA9/PA10 电脑端 + USART3 PB10/PB11 蓝牙端 @ 115200 8N1)
  Telemetry_Init();
  HAL_Delay(100);

  Telemetry_Printf("\r\n\r\n");
  Telemetry_Printf("============================================================\r\n");
  Telemetry_Printf(" STM32F103 + AHT20 温湿度传感器 + PB-03F 蓝牙无线测试工程\r\n");
  Telemetry_Printf(" 硬件总线配置:\r\n");
  Telemetry_Printf("   * AHT20 I2C : SCL = PB6, SDA = PB7\r\n");
  Telemetry_Printf("   * PB-03F 蓝牙: USART3 (PB10=TX, PB11=RX) @ 115200 8N1\r\n");
  Telemetry_Printf("   * USB-TTL 调试: USART1 (PA9=TX, PA10=RX) @ 115200 8N1\r\n");
  Telemetry_Printf("============================================================\r\n");

  // 3. 初始化 AHT20 传感器
  uint8_t aht_ok = AHT20_Init();
  if (aht_ok)
  {
    HAL_GPIO_WritePin(LED_PORT, LED_READY_PIN, GPIO_PIN_SET);
    Telemetry_Printf("[AHT20] >>> 成功检测到 AHT20 传感器 (I2C 0x38 ACK 正常，校准完成) <<<\r\n");
    // 开机就绪双音
    Buzzer_Tone(1047, 60);
    HAL_Delay(30);
    Buzzer_Tone(1319, 80);
  }
  else
  {
    Telemetry_Printf("[AHT20] >>> 警告: 未收到 AHT20 传感器应答！请检查 PB6(SCL) 和 PB7(SDA) 接线及供电。 <<<\r\n");
    Buzzer_Tone(300, 300);
  }

  uint32_t last_report_tick = 0;
  uint32_t report_interval_ms = 1000; // 默认 1 秒上报一次
  float temp = 0.0f;
  float humi = 0.0f;
  float baseline_humi = 50.0f;
  char ble_cmd[64];

  while (1)
  {
    uint32_t now = HAL_GetTick();

    // 1. 周期性采集与双通道推送 (电脑串口 + 手机蓝牙同步输出)
    if (now - last_report_tick >= report_interval_ms)
    {
      last_report_tick = now;
      HAL_GPIO_TogglePin(LED_PORT, LED_BEAT_PIN); // 心跳闪烁

      if (aht_ok)
      {
        uint8_t read_ok = AHT20_Read(&temp, &humi);
        if (read_ok)
        {
          // 格式化输出 (保留 1 位小数)
          Telemetry_Printf("[AHT20] 温度: %.1f C | 湿度: %.1f %%RH\r\n", temp, humi);

          // 吹气 / 哈气检测 (相对湿度激增判定)
          if (humi > baseline_humi + 12.0f || humi >= 75.0f)
          {
            Telemetry_Printf(">>> [吹气检测] 检测到人为哈气/湿气突增! (当前湿度: %.1f%%) <<<\r\n", humi);
            Buzzer_Tone(1568, 50); // 欢快鸟鸣短音
          }
          else
          {
            // 平滑跟踪日常环境湿度基准
            baseline_humi = (baseline_humi * 3.0f + humi) / 4.0f;
          }
        }
        else
        {
          static uint8_t fail_cnt = 0;
          fail_cnt++;
          Telemetry_Printf("[AHT20] 数据读取失败 (%d/3)...\r\n", fail_cnt);
          if (fail_cnt >= 3)
          {
            fail_cnt = 0;
            aht_ok = 0;
            HAL_GPIO_WritePin(LED_PORT, LED_READY_PIN, GPIO_PIN_RESET);
            Telemetry_Printf("[AHT20] 连续3次读取失败，切换至重新探测状态...\r\n");
          }
        }

      }
      else
      {
        // 传感器未就绪时尝试热插拔重连
        aht_ok = AHT20_Init();
        if (aht_ok)
        {
          HAL_GPIO_WritePin(LED_PORT, LED_READY_PIN, GPIO_PIN_SET);
          Telemetry_Printf("[AHT20] >>> AHT20 热插拔重连成功！ <<<\r\n");
          Buzzer_Tone(1319, 100);
        }
        else
        {
          Telemetry_Printf("[AHT20] 正在等待传感器接入 PB6(SCL) / PB7(SDA)...\r\n");
        }
      }
    }

    // 2. 监听蓝牙接收通道 (支持手机通过 BLE 发送指令操控单片机)
    if (Telemetry_ReadBLERx(ble_cmd, sizeof(ble_cmd)))
    {
      // 忽略由自身串口广播回环产生的数据
      if (ble_cmd[0] == '[' || ble_cmd[0] == '>' || ble_cmd[0] == '=' || strlen(ble_cmd) == 0)
      {
        continue;
      }

      if (strcmp(ble_cmd, "1") == 0)
      {
        Buzzer_Tone(1047, 100);
        Telemetry_Printf("[BLE ACK] 蜂鸣器发声 (1047Hz) 执行完毕\r\n");
      }
      else if (strcmp(ble_cmd, "0") == 0)
      {
        Buzzer_Tone(440, 100);
        Telemetry_Printf("[BLE ACK] 蜂鸣器发声 (440Hz) 执行完毕\r\n");
      }
      else if (strcasecmp(ble_cmd, "READ") == 0 || strcasecmp(ble_cmd, "GET") == 0)
      {
        if (AHT20_Read(&temp, &humi))
        {
          Telemetry_Printf("[BLE ACK] 实时温湿度 -> 温度: %.1f C, 湿度: %.1f %%\r\n", temp, humi);
        }
      }
      else if (strcasecmp(ble_cmd, "FAST") == 0)
      {
        report_interval_ms = 300;
        Telemetry_Printf("[BLE ACK] 上报频率调至 300ms 快速推流\r\n");
      }
      else if (strcasecmp(ble_cmd, "SLOW") == 0)
      {
        report_interval_ms = 1000;
        Telemetry_Printf("[BLE ACK] 上报频率调至 1000ms 标准间隔\r\n");
      }
    }
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16; // 64MHz
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

void Error_Handler(void)
{
  while (1);
}
