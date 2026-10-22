#include "servo.h"
#include "telemetry.h"

static float    current_angle = 90.0f;
static uint16_t current_pulse = SERVO_MID_PULSE_US;

static uint8_t  sweep_enabled = 0;
static float    sweep_target  = 180.0f;
static float    sweep_step    = 2.0f;
static uint32_t last_sweep_tick = 0;

/* ---------------- 舵机底层 PWM 驱动 (PB0 / TIM3_CH3 @ 50Hz) ---------------- */
void Servo_Init(void)
{
  // 1. 开启外设时钟 (GPIOB, AFIO, TIM3)
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_RCC_TIM3_CLK_ENABLE();

  // 2. 配置 PB0 为复用推挽输出 (AF_PP 50MHz) -> 对应 TIM3_CH3
  // STM32F103 CRL: CNF=10 (AF_PP), MODE=11 (50MHz) -> 0b1011 = 0x0B
  GPIOB->CRL = (GPIOB->CRL & ~0x0FU) | 0x0BU;

  // 3. 配置 TIM3 计数器时钟为 1MHz (1 个计数值 = 1 微秒精确度)
  // 当前系统主频 64MHz，APB1 分频为 2，故 APB1 内部定时器倍频为 64MHz
  // PSC = 64 - 1 = 63 -> 计数频率 = 64MHz / 64 = 1MHz (1us/tick)
  TIM3->PSC = 64 - 1;

  // 4. 配置自动重装载周期为 20ms (50Hz 标准舵机控制频率)
  // ARR = 20000 - 1 = 19999 -> 周期 20,000us = 20ms
  TIM3->ARR = 20000 - 1;

  // 5. 设置通道 3 初始占空比 (默认 1500us 居中 90 度)
  TIM3->CCR3 = SERVO_MID_PULSE_US;

  // 6. 配置通道 3 为 PWM 模式 1 (计数值 < CCR3 时输出高电平)，开启预装载
  TIM3->CCMR2 &= ~(TIM_CCMR2_OC3M | TIM_CCMR2_OC3PE);
  TIM3->CCMR2 |= (6U << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE;

  // 7. 开启通道 3 输出 (高电平有效)
  TIM3->CCER |= TIM_CCER_CC3E;

  // 8. 开启 ARPE 预装载并启动计数器
  TIM3->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;
  TIM3->EGR |= TIM_EGR_UG;

  // 初始化状态同步
  Servo_SetAngle(90.0f);
  Telemetry_Printf("[SERVO] PB0 (TIM3_CH3) 硬件 50Hz 纯微秒级 PWM 发生器初始化完毕 (1us精度)\r\n");
}

/* 设置舵机目标角度 (0.0° ~ 180.0°) */
void Servo_SetAngle(float angle)
{
  if (angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
  if (angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;

  current_angle = angle;
  // 角度映射到 500us ~ 2500us
  current_pulse = SERVO_MIN_PULSE_US + (uint16_t)((angle / 180.0f) * (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US));

  TIM3->CCR3 = current_pulse;
}

/* 直接设置舵机高电平脉宽 (500us ~ 2500us) */
void Servo_SetPulse(uint16_t pulse_us)
{
  if (pulse_us < SERVO_MIN_PULSE_US) pulse_us = SERVO_MIN_PULSE_US;
  if (pulse_us > SERVO_MAX_PULSE_US) pulse_us = SERVO_MAX_PULSE_US;

  current_pulse = pulse_us;
  // 脉宽映射回角度
  current_angle = ((float)(pulse_us - SERVO_MIN_PULSE_US) / (float)(SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US)) * 180.0f;

  TIM3->CCR3 = current_pulse;
}

float Servo_GetAngle(void)
{
  return current_angle;
}

uint16_t Servo_GetPulse(void)
{
  return current_pulse;
}

void Servo_SetSweepMode(uint8_t enable)
{
  sweep_enabled = enable ? 1 : 0;
  if (sweep_enabled)
  {
    Telemetry_Printf("[SERVO SWEEP] 自动往复扫动模式: 已开启 (0° <-> 180°)\r\n");
  }
  else
  {
    Telemetry_Printf("[SERVO SWEEP] 自动往复扫动模式: 已停止 (当前锁定 %.1f°)\r\n", current_angle);
  }
}

uint8_t Servo_GetSweepMode(void)
{
  return sweep_enabled;
}

/* 在主循环中周期性调用，提供丝滑匀速的往复巡航摆动 */
void Servo_UpdateSweep(void)
{
  if (!sweep_enabled) return;

  uint32_t now = HAL_GetTick();
  if (now - last_sweep_tick >= 20) // 每 20ms (对应 1 帧 50Hz PWM) 步进一次
  {
    last_sweep_tick = now;

    if (sweep_target > current_angle)
    {
      current_angle += sweep_step;
      if (current_angle >= SERVO_MAX_ANGLE)
      {
        current_angle = SERVO_MAX_ANGLE;
        sweep_target = SERVO_MIN_ANGLE; // 转向
      }
    }
    else
    {
      current_angle -= sweep_step;
      if (current_angle <= SERVO_MIN_ANGLE)
      {
        current_angle = SERVO_MIN_ANGLE;
        sweep_target = SERVO_MAX_ANGLE; // 转向
      }
    }

    Servo_SetAngle(current_angle);
  }
}
