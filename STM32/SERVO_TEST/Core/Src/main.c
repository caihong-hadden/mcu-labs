#include "main.h"
#include "servo.h"
#include "scope_art.h"
#include "telemetry.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void SystemClock_Config(void);

#define BUZZER_PIN       GPIO_PIN_7
#define BUZZER_PORT      GPIOA
#define LED_HEART_PIN    GPIO_PIN_1
#define LED_PORT         GPIOB

// 板载按键引脚映射
#define BTN_JUMP_PIN     GPIO_PIN_0   // PA0 (切换为 心电图 / 舵机归中)
#define BTN_JUMP_PORT    GPIOA
#define BTN_LEFT_PIN     GPIO_PIN_12  // PB12 (切换为 城市天际线与城堡)
#define BTN_LEFT_PORT    GPIOB
#define BTN_RIGHT_PIN    GPIO_PIN_13  // PB13 (切换为 超级马里奥关卡)
#define BTN_RIGHT_PORT   GPIOB
#define BTN_MODE_PIN     GPIO_PIN_9   // PB9 (一键轮换下一幅画 / 自动扫动)
#define BTN_MODE_PORT    GPIOB

/* 蜂鸣器发声 */
static void Buzzer_Tone(uint16_t freq_hz, uint16_t duration_ms)
{
  if (freq_hz == 0 || duration_ms == 0) return;
  uint32_t half_period_us = 500000 / freq_hz;
  uint32_t cycles = ((uint32_t)freq_hz * duration_ms) / 1000;
  for (uint32_t i = 0; i < cycles; i++)
  {
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
    for (volatile int d = 0; d < half_period_us * 8; d++);
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
    for (volatile int d = 0; d < half_period_us * 8; d++);
  }
}

/* 初始化板载按键与指示灯 */
static void Hardware_IO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // 1. 蜂鸣器 (PA7) 推挽输出
  GPIO_InitStruct.Pin = BUZZER_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BUZZER_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);

  // 2. 心跳灯 (PB1) 推挽输出
  GPIO_InitStruct.Pin = LED_HEART_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(LED_PORT, LED_HEART_PIN, GPIO_PIN_RESET);

  // 3. 板载物理按键配置为内部上拉输入
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;

  GPIO_InitStruct.Pin = BTN_JUMP_PIN;
  HAL_GPIO_Init(BTN_JUMP_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = BTN_LEFT_PIN | BTN_RIGHT_PIN | BTN_MODE_PIN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* 按键轮询检测与去抖 */
static void Scan_Buttons(void)
{
  static uint32_t last_btn_tick = 0;
  uint32_t now = HAL_GetTick();
  if (now - last_btn_tick < 200) return; // 200ms 防抖

  // 1. PA0 (跳跃键) -> 切到画画 1: 💓 真实心电图
  if (HAL_GPIO_ReadPin(BTN_JUMP_PORT, BTN_JUMP_PIN) == GPIO_PIN_RESET)
  {
    last_btn_tick = now;
    ScopeArt_SetMode(ART_MODE_ECG);
    Buzzer_Tone(1047, 40);
  }
  // 2. PB12 (向左键) -> 切到画画 2: 🏰 城市天际线与城堡
  else if (HAL_GPIO_ReadPin(BTN_LEFT_PORT, BTN_LEFT_PIN) == GPIO_PIN_RESET)
  {
    last_btn_tick = now;
    ScopeArt_SetMode(ART_MODE_SKYLINE);
    Buzzer_Tone(880, 40);
  }
  // 3. PB13 (向右键) -> 切到画画 3: 🍄 超级马里奥关卡
  else if (HAL_GPIO_ReadPin(BTN_RIGHT_PORT, BTN_RIGHT_PIN) == GPIO_PIN_RESET)
  {
    last_btn_tick = now;
    ScopeArt_SetMode(ART_MODE_MARIO);
    Buzzer_Tone(1319, 40);
  }
  // 4. PB9 (冲刺/确认键) -> 循环切换下一幅画 (心电图 -> 天际线 -> 马里奥 -> 爱心 -> 舵机)
  else if (HAL_GPIO_ReadPin(BTN_MODE_PORT, BTN_MODE_PIN) == GPIO_PIN_RESET)
  {
    last_btn_tick = now;
    ScopeArt_NextMode();
    Buzzer_Tone(1568, 50);
  }
}

/* 解析串口与蓝牙发来的控制指令 */
static void Process_Command(const char *cmd)
{
  if (!cmd || strlen(cmd) == 0) return;
  if (cmd[0] == '[' || cmd[0] == '>' || cmd[0] == '=') return;

  Telemetry_Printf("[CMD RECV] 收到控制指令: '%s'\r\n", cmd);

  // 1. 示波器画画切换指令
  if (strcmp(cmd, "1") == 0 || strcasecmp(cmd, "ECG") == 0)
  {
    ScopeArt_SetMode(ART_MODE_ECG);
    Buzzer_Tone(1047, 50);
    return;
  }
  if (strcmp(cmd, "2") == 0 || strcasecmp(cmd, "SKY") == 0 || strcasecmp(cmd, "SKYLINE") == 0)
  {
    ScopeArt_SetMode(ART_MODE_SKYLINE);
    Buzzer_Tone(1175, 50);
    return;
  }
  if (strcmp(cmd, "3") == 0 || strcasecmp(cmd, "MARIO") == 0)
  {
    ScopeArt_SetMode(ART_MODE_MARIO);
    Buzzer_Tone(1319, 50);
    return;
  }
  if (strcmp(cmd, "4") == 0 || strcasecmp(cmd, "HEART") == 0)
  {
    ScopeArt_SetMode(ART_MODE_HEART);
    Buzzer_Tone(1568, 50);
    return;
  }
  if (strcmp(cmd, "5") == 0 || strcasecmp(cmd, "SERVO") == 0)
  {
    ScopeArt_SetMode(ART_MODE_SERVO);
    Buzzer_Tone(880, 50);
    return;
  }

  // 2. 舵机模式下的操控指令
  if (ScopeArt_GetMode() == ART_MODE_SERVO)
  {
    char *endptr;
    float val = strtof(cmd, &endptr);
    if (endptr != cmd && *endptr == '\0')
    {
      Servo_SetSweepMode(0);
      Servo_SetAngle(val);
      Buzzer_Tone(1047, 50);
      Telemetry_Printf("[ACK] 舵机定位至: %.1f° (%d us)\r\n", Servo_GetAngle(), Servo_GetPulse());
      return;
    }

    if (cmd[0] == 'P' || cmd[0] == 'p')
    {
      int pulse = atoi(cmd + 1);
      if (pulse >= SERVO_MIN_PULSE_US && pulse <= SERVO_MAX_PULSE_US)
      {
        Servo_SetSweepMode(0);
        Servo_SetPulse((uint16_t)pulse);
        Buzzer_Tone(1319, 50);
        Telemetry_Printf("[ACK] 舵机脉宽设定: %d us (%.1f°)\r\n", pulse, Servo_GetAngle());
        return;
      }
    }

    if (strcmp(cmd, "+") == 0)
    {
      Servo_SetSweepMode(0);
      Servo_SetAngle(Servo_GetAngle() + 5.0f);
      Telemetry_Printf("[ACK] 角度 +5° -> %.1f°\r\n", Servo_GetAngle());
      return;
    }
    if (strcmp(cmd, "-") == 0)
    {
      Servo_SetSweepMode(0);
      Servo_SetAngle(Servo_GetAngle() - 5.0f);
      Telemetry_Printf("[ACK] 角度 -5° -> %.1f°\r\n", Servo_GetAngle());
      return;
    }
    if (strcasecmp(cmd, "MID") == 0 || strcasecmp(cmd, "CENTER") == 0)
    {
      Servo_SetSweepMode(0);
      Servo_SetAngle(90.0f);
      Buzzer_Tone(1047, 50);
      return;
    }
    if (strcasecmp(cmd, "SWEEP") == 0 || strcasecmp(cmd, "S") == 0)
    {
      Servo_SetSweepMode(1);
      Buzzer_Tone(1568, 80);
      return;
    }
    if (strcasecmp(cmd, "STOP") == 0)
    {
      Servo_SetSweepMode(0);
      Buzzer_Tone(587, 80);
      return;
    }
  }

  // 帮助菜单
  if (strcasecmp(cmd, "HELP") == 0 || strcmp(cmd, "?") == 0)
  {
    Telemetry_Printf("\r\n--- 示波器画画与控制指令手册 ---\r\n");
    Telemetry_Printf(" [输入 1 / ECG]     : 💓 真实医院心电监护仪 (P-Q-R-S-T波 + 滴滴心率声)\r\n");
    Telemetry_Printf(" [输入 2 / SKY]     : 🏰 宏伟城市天际线与古典城堡 (高楼、城堞、尖塔、大桥)\r\n");
    Telemetry_Printf(" [输入 3 / MARIO]   : 🍄 超级马里奥关卡世界 (水管、浮空砖块、终点旗杆)\r\n");
    Telemetry_Printf(" [输入 4 / HEART]   : 💖 动态呼吸脉动爱心 (高频脉冲包络)\r\n");
    Telemetry_Printf(" [输入 5 / SERVO]   : ⚙️ 切换回舵机 PWM 角度控制模式\r\n");
    Telemetry_Printf(" [板载按键快速切换] : PA0=心电图, PB12=城堡天际线, PB13=马里奥, PB9=轮换下一幅\r\n");
    Telemetry_Printf("--------------------------------\r\n\r\n");
    return;
  }

  Telemetry_Printf("[ERR] 未知指令 '%s'，输入 HELP 或 ? 查看可用画画指令\r\n", cmd);
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  // 1. 初始化双通道通信 (USART1 调试串口 + USART3 蓝牙 @ 115200 8N1)
  Telemetry_Init();
  HAL_Delay(50);

  Telemetry_Printf("\r\n\r\n");
  Telemetry_Printf("============================================================\r\n");
  Telemetry_Printf(" STM32F103 + PB0 示波器艺术画廊 & 舵机双用驱动引擎\r\n");
  Telemetry_Printf(" 示波器观测最佳设置参数:\r\n");
  Telemetry_Printf("   * 示波器通道 1 探头: 接 STM32 单片机 PB0 引脚 (探头地线接 GND)\r\n");
  Telemetry_Printf("   * 水平时基 (Timebase): 调至 1.0 ms / div (或 500 us / div)\r\n");
  Telemetry_Printf("   * 垂直刻度 (Voltage) : 调至 1.0 V / div (0V 基线对准屏幕下部第二格)\r\n");
  Telemetry_Printf("   * 触发模式 (Trigger) : 边沿触发 (Edge) -> 上升沿 (Rising) -> 电平约 1.5V\r\n");
  Telemetry_Printf("============================================================\r\n");

  // 2. 初始化按键、蜂鸣器与指示灯
  Hardware_IO_Init();

  // 3. 初始化示波器画画引擎 (默认启动 💓 心电图 模式)
  ScopeArt_Init();

  // 开机欢迎音律
  Buzzer_Tone(1047, 40);
  HAL_Delay(20);
  Buzzer_Tone(1319, 40);
  HAL_Delay(20);
  Buzzer_Tone(1568, 60);

  uint32_t last_report_tick = 0;
  char ble_cmd[64];

  while (1)
  {
    uint32_t now = HAL_GetTick();

    // 1. 扫描板载按键 (支持随时切换画画)
    Scan_Buttons();

    // 2. 核心：在 PB0 引脚上以极速无抖动时序渲染一帧示波器画面！
    ScopeArt_RenderFrame();

    // 3. 周期性状态上报 (每 1 秒在串口/蓝牙打印一次当前画面)
    if (now - last_report_tick >= 1000)
    {
      last_report_tick = now;
      HAL_GPIO_TogglePin(LED_PORT, LED_HEART_PIN);

      if (ScopeArt_GetMode() == ART_MODE_SERVO)
      {
        Telemetry_Printf("[SERVO] 角度: %5.1f° | 脉宽: %4d us | 状态: %s\r\n",
                         Servo_GetAngle(), Servo_GetPulse(),
                         Servo_GetSweepMode() ? "自动往复巡航中" : "角度锁定");
      }
      else
      {
        Telemetry_Printf("[示波器画廊] 正在 PB0 绘制: %s | 探头时基设为 1.0ms/div 即可清晰定格！\r\n",
                         ScopeArt_GetModeName());
      }
    }

    // 4. 监听串口与蓝牙发来的控制指令
    if (Telemetry_ReadBLERx(ble_cmd, sizeof(ble_cmd)))
    {
      Process_Command(ble_cmd);
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
