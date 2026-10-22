#include "scope_art.h"
#include "servo.h"
#include "telemetry.h"
#include <math.h>

static ScopeArtMode_t current_mode = ART_MODE_ECG;
static uint32_t heart_beat_counter = 0;
static float    heart_pulse_phase  = 0.0f;

/* 精确微秒级 DWT 延时 (基于 64MHz 内部硬件时钟周期计数器) */
static inline void DWT_Init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline void Delay_us(uint32_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t target = us * 64; // 64MHz 下 64 个 cycle = 1 us
  while ((DWT->CYCCNT - start) < target);
}

static inline void PB0_High(void) { GPIOB->BSRR = GPIO_PIN_0; }
static inline void PB0_Low(void)  { GPIOB->BRR  = GPIO_PIN_0; }

/* 模拟中间灰阶电平 (利用高频占空比在示波器上呈现阶梯/半高波形) */
static void PB0_Level_us(uint8_t duty_percent, uint32_t duration_us)
{
  if (duty_percent == 0)
  {
    PB0_Low();
    Delay_us(duration_us);
    return;
  }
  if (duty_percent >= 100)
  {
    PB0_High();
    Delay_us(duration_us);
    return;
  }

  uint32_t cycles = duration_us / 2; // 500kHz 载波
  for (uint32_t i = 0; i < cycles; i++)
  {
    PB0_High();
    Delay_us((2 * duty_percent) / 100);
    PB0_Low();
    Delay_us(2 - (2 * duty_percent) / 100);
  }
}

void ScopeArt_Init(void)
{
  DWT_Init();

  // 配置 PB0 为标准推挽输出 (50MHz)
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  PB0_Low();

  current_mode = ART_MODE_ECG; // 默认启动心电图艺术模式
  Telemetry_Printf("[SCOPE ART] 示波器画画模式已激活！探头接 PB0，示波器水平时基调至 1.0 ms/div，触发设为上升沿。\r\n");
}

void ScopeArt_SetMode(ScopeArtMode_t mode)
{
  current_mode = mode;
  if (mode == ART_MODE_SERVO)
  {
    // 切换回舵机驱动 (TIM3_CH3 PWM 模式)
    Servo_Init();
    Telemetry_Printf("[SCOPE ART] 模式已切换至 -> ⚙️ 5. 舵机控制模式 (TIM3 50Hz PWM)\r\n");
  }
  else
  {
    // 重新配置 PB0 为 GPIO 推挽输出供示波器绘制
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    PB0_Low();

    Telemetry_Printf("[SCOPE ART] 模式已切换至 -> %s (示波器时基建议: 1.0ms/div)\r\n",
                     ScopeArt_GetModeName());
  }
}

ScopeArtMode_t ScopeArt_GetMode(void)
{
  return current_mode;
}

const char* ScopeArt_GetModeName(void)
{
  switch (current_mode)
  {
    case ART_MODE_ECG:     return "💓 1. 真实医院心电监护仪 (ECG P-Q-R-S-T 波)";
    case ART_MODE_SKYLINE: return "🏰 2. 宏伟城市天际线与古典城堡 (Skyline & Castle)";
    case ART_MODE_MARIO:   return "🍄 3. 超级马里奥横版关卡世界 (Mario Level)";
    case ART_MODE_HEART:   return "💖 4. 动态呼吸脉动爱心包络 (Pulsing Heart)";
    case ART_MODE_SERVO:   return "⚙️ 5. 舵机硬件控制测试模式 (Servo PWM)";
    default:               return "未知模式";
  }
}

void ScopeArt_NextMode(void)
{
  uint8_t m = (uint8_t)current_mode + 1;
  if (m > 5) m = 1;
  ScopeArt_SetMode((ScopeArtMode_t)m);
}

/* ---------------- 艺术画 1: 💓 真实医院心电图 (ECG) ---------------- */
static void Render_ECG(void)
{
  // 1. 同步基线 (2000 us = 2ms) -> 供示波器稳定捕获触发上升沿
  PB0_Low();
  Delay_us(2000);

  // 2. P 波 (心房收缩：圆润平缓鼓起，幅度约 35%)
  PB0_Level_us(35, 600);

  // 3. PR 间期 (平坦等电位线)
  PB0_Low();
  Delay_us(400);

  // 4. Q 波 (轻微负向切迹，保持低平)
  PB0_Low();
  Delay_us(150);

  // 5. R 波 (心室激动尖峰：以 3.3V 强脉冲直冲天际！)
  PB0_High();
  Delay_us(350); // 极窄锐利峰值

  // 6. S 波 (急剧跌落至 0V)
  PB0_Low();
  Delay_us(250);

  // 7. ST 段 (心室复极初始平线)
  PB0_Low();
  Delay_us(500);

  // 8. T 波 (心室复极波：宽大圆丘，幅度约 45%)
  PB0_Level_us(45, 1200);

  // 9. 舒张期基线静止 (剩余时间补足 10ms 帧周期)
  PB0_Low();
  Delay_us(4550);

  // 同步心跳蜂鸣声 (每 75 帧约 750ms 发出一声真实医疗滴滴声)
  heart_beat_counter++;
  if (heart_beat_counter >= 75)
  {
    heart_beat_counter = 0;
    // 蜂鸣器发出一声清脆的高频提示音
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
    Delay_us(400);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
  }
}

/* ---------------- 艺术画 2: 🏰 宏伟城市天际线与古典城堡 ---------------- */
static void Render_Skyline(void)
{
  // 1. 同步引导底线 (1.5ms)
  PB0_Low();
  Delay_us(1500);

  // 2. 古典城堡堡垒区
  PB0_High(); Delay_us(300); // 外围城墙
  PB0_Low();  Delay_us(100); // 护城门缺口

  // 4 座对称城堞 (箭垛 Crenellations)
  for (int i = 0; i < 4; i++)
  {
    PB0_High(); Delay_us(80);
    PB0_Low();  Delay_us(80);
  }

  // 城堡中央主塔与旗杆
  PB0_High(); Delay_us(500); // 主塔台
  PB0_Low();  Delay_us(40);
  PB0_High(); Delay_us(40);  // 塔尖细针尖顶 (Needle Spire)
  PB0_Low();  Delay_us(40);
  PB0_High(); Delay_us(200); // 塔楼右翼
  PB0_Low();  Delay_us(200);

  // 3. 现代大都市摩天大楼群
  // 帝国大厦式阶梯建筑
  PB0_High(); Delay_us(200); // 裙楼
  PB0_High(); Delay_us(350); // 主楼
  PB0_Low();  Delay_us(50);
  PB0_High(); Delay_us(50);  // 避雷针天线
  PB0_Low();  Delay_us(150);

  // 东方明珠 / 广播电视塔
  PB0_High(); Delay_us(150); // 塔座
  PB0_Low();  Delay_us(60);
  PB0_High(); Delay_us(250); // 下球体
  PB0_Low();  Delay_us(60);
  PB0_High(); Delay_us(180); // 上球体
  PB0_Low();  Delay_us(40);
  PB0_High(); Delay_us(40);  // 发射天线极细峰
  PB0_Low();  Delay_us(200);

  // 玻璃幕墙大厦 (密排窗户切迹)
  for (int i = 0; i < 5; i++)
  {
    PB0_High(); Delay_us(90);
    PB0_Low();  Delay_us(30);
  }
  PB0_Low(); Delay_us(200);

  // 4. 跨海斜拉悬索大桥
  PB0_High(); Delay_us(150); // 左主塔
  PB0_Low();  Delay_us(80);
  // 悬索下垂渐变脉冲
  PB0_High(); Delay_us(50); PB0_Low(); Delay_us(60);
  PB0_High(); Delay_us(70); PB0_Low(); Delay_us(60);
  PB0_High(); Delay_us(100); PB0_Low(); Delay_us(60);
  PB0_High(); Delay_us(150); // 右主塔

  // 5. 尾部平坦基线
  PB0_Low();
  Delay_us(3000);
}

/* ---------------- 艺术画 3: 🍄 超级马里奥横版关卡世界 ---------------- */
static void Render_Mario(void)
{
  // 1. 同步地面基准线 (1.5ms)
  PB0_Low();
  Delay_us(1500);

  // 2. 矮水管 (Warp Pipe 1)
  PB0_High(); Delay_us(120); // 水管上沿凸缘 (Pipe Lip)
  PB0_Low();  Delay_us(20);
  PB0_High(); Delay_us(280); // 水管管身
  PB0_Low();  Delay_us(400); // 平地

  // 3. 悬空砖块与问号方块群 (Floating Blocks)
  // 砖块 1
  PB0_High(); Delay_us(150); PB0_Low(); Delay_us(80);
  // 问号方块 (Lucky ?)
  PB0_High(); Delay_us(220); PB0_Low(); Delay_us(80);
  // 砖块 2
  PB0_High(); Delay_us(150);
  PB0_Low();  Delay_us(500);

  // 4. 高水管 (Warp Pipe 2)
  PB0_High(); Delay_us(160); // 凸缘
  PB0_Low();  Delay_us(20);
  PB0_High(); Delay_us(550); // 高水管身
  PB0_Low();  Delay_us(450);

  // 5. 四级冲刺过关阶梯 (Staircase)
  PB0_High(); Delay_us(120); PB0_Low(); Delay_us(30);
  PB0_High(); Delay_us(220); PB0_Low(); Delay_us(30);
  PB0_High(); Delay_us(320); PB0_Low(); Delay_us(30);
  PB0_High(); Delay_us(420); PB0_Low(); Delay_us(250);

  // 6. 终点旗杆 (The Flagpole) 与旗帜
  PB0_High(); Delay_us(50);  // 极细高耸旗杆
  PB0_Low();  Delay_us(20);
  PB0_High(); Delay_us(200); // 飘扬旗帜方块
  PB0_Low();  Delay_us(300);

  // 7. 终点城堡
  PB0_High(); Delay_us(300); // 城堡塔
  PB0_Low();  Delay_us(50);
  PB0_High(); Delay_us(100); // 箭垛
  PB0_Low();  Delay_us(50);
  PB0_High(); Delay_us(100);

  // 8. 关卡结束基线
  PB0_Low();
  Delay_us(2500);
}

/* ---------------- 艺术画 4: 💖 动态呼吸脉动爱心包络 ---------------- */
static void Render_Heart(void)
{
  heart_pulse_phase += 0.05f;
  if (heart_pulse_phase > 6.28318f) heart_pulse_phase = 0.0f;
  float scale = 0.85f + 0.15f * sinf(heart_pulse_phase); // 呼吸膨胀系数

  // 1. 同步低电平 (1.5ms)
  PB0_Low();
  Delay_us(1500);

  // 2. 心形包络高密度脉宽扫描 (60 根微细光栅形成饱满爱心轮廓)
  for (int x = -30; x <= 30; x++)
  {
    float fx = (float)x / 30.0f;
    float abs_x = fabsf(fx);
    // 爱心上部双峰 + 下部收尖数学曲线
    float top_lobe = sqrtf(1.0f - (abs_x - 0.5f) * (abs_x - 0.5f));
    float heart_val = (top_lobe - abs_x * 0.4f) * scale;
    if (heart_val < 0.0f) heart_val = 0.0f;

    uint32_t width_us = (uint32_t)(heart_val * 90.0f);
    if (width_us > 0)
    {
      PB0_High();
      Delay_us(width_us);
      PB0_Low();
      Delay_us(95 - width_us);
    }
    else
    {
      PB0_Low();
      Delay_us(95);
    }
  }

  // 3. 补足帧周期
  PB0_Low();
  Delay_us(2800);
}

/* 统一帧渲染入口 */
void ScopeArt_RenderFrame(void)
{
  switch (current_mode)
  {
    case ART_MODE_ECG:     Render_ECG();     break;
    case ART_MODE_SKYLINE: Render_Skyline(); break;
    case ART_MODE_MARIO:   Render_Mario();   break;
    case ART_MODE_HEART:   Render_Heart();   break;
    case ART_MODE_SERVO:   Servo_UpdateSweep(); break;
    default: break;
  }
}
