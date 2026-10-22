#include "mario.h"
#include "oled.h"
#include "telemetry.h"
#include "ps2.h"
#include <string.h>
#include <stdlib.h>

/* ====================================================================
 * 8-Bit 像素经典角色与游戏物件点阵表
 * ==================================================================== */
static const uint8_t bmp_mario_stand[] = {
  0x0F, 0x00, 0x1F, 0xE0, 0x1C, 0x40, 0x2E, 0x70, 0x2E, 0x20, 0x30, 0x00,
  0x0F, 0x00, 0x1B, 0x00, 0x3B, 0x80, 0x79, 0xE0, 0x69, 0x60, 0x60, 0x60,
  0x0F, 0x00, 0x1B, 0x00, 0x3B, 0x80
};

static const uint8_t bmp_mario_run[] = {
  0x0F, 0x00, 0x1F, 0xE0, 0x1C, 0x40, 0x2E, 0x70, 0x2E, 0x20, 0x30, 0x00,
  0x0F, 0x00, 0x1F, 0x00, 0x3F, 0x00, 0x69, 0x60, 0x0F, 0x00, 0x1B, 0x00,
  0x24, 0x80, 0x70, 0xE0, 0x00, 0x00
};

static const uint8_t bmp_mario_jump[] = {
  0x0F, 0x30, 0x1F, 0xE0, 0x1C, 0x40, 0x2E, 0x70, 0x2E, 0x20, 0x30, 0x00,
  0x0F, 0x00, 0x1F, 0x00, 0x3F, 0x30, 0x69, 0x30, 0x0F, 0x00, 0x1B, 0x00,
  0x38, 0xC0, 0x00, 0x70, 0x00, 0x00
};

static const uint8_t bmp_goomba1[] = {
  0x0F, 0x00, 0x3F, 0xC0, 0x7F, 0xE0, 0xE9, 0x70, 0xE9, 0x70, 0xFF, 0xF0,
  0x7F, 0xE0, 0x1F, 0x80, 0x30, 0xC0, 0x70, 0xE0
};

static const uint8_t bmp_goomba2[] = {
  0x0F, 0x00, 0x3F, 0xC0, 0x7F, 0xE0, 0xE9, 0x70, 0xE9, 0x70, 0xFF, 0xF0,
  0x7F, 0xE0, 0x1F, 0x80, 0x0C, 0x30, 0x0E, 0x70
};

static const uint8_t bmp_goomba_flat[] = {
  0x00, 0x00, 0x3F, 0xC0, 0x7F, 0xE0, 0xFF, 0xF0, 0xE0, 0x70
};

static const uint8_t bmp_qblock[] = {
  0xFF, 0xC0, 0x87, 0x80, 0xB6, 0xC0, 0x81, 0x80, 0x83, 0x00, 0x86, 0x00,
  0x80, 0x00, 0x86, 0x00, 0x86, 0x00, 0xFF, 0xC0
};

static const uint8_t bmp_eblock[] = {
  0xFF, 0xC0, 0xC0, 0xC0, 0xA1, 0x40, 0x92, 0x40, 0x8C, 0x40, 0x8C, 0x40,
  0x92, 0x40, 0xA1, 0x40, 0xC0, 0xC0, 0xFF, 0xC0
};

static const uint8_t bmp_coin[] = {
  0x3C, 0x7E, 0xD7, 0xD7, 0xD7, 0xD7, 0x7E, 0x3C
};

static const uint8_t bmp_cloud[] = {
  0x03, 0xC0, 0x0C, 0x30, 0x18, 0x18, 0x70, 0x0E, 0xC0, 0x03, 0xC0, 0x03,
  0x70, 0x0E, 0x1F, 0xF8, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t bmp_bush[] = {
  0x07, 0xE0, 0x18, 0x18, 0x30, 0x0C, 0x60, 0x06, 0x60, 0x06, 0x30, 0x0C,
  0x7F, 0xFE, 0x7F, 0xFE
};

/* ====================================================================
 * 蜂鸣器微秒发声驱动 (PA7 - 适配 64MHz 极速主频)
 * ==================================================================== */
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

static void SFX_Jump(void)
{
  Buzzer_Tone(494, 15);
  Buzzer_Tone(659, 20);
}

static void SFX_Coin(void)
{
  Buzzer_Tone(988, 30);
  Buzzer_Tone(1319, 50);
}

static void SFX_Stomp(void)
{
  Buzzer_Tone(392, 15);
  Buzzer_Tone(262, 25);
}

static void SFX_Bump(void)
{
  Buzzer_Tone(150, 15);
}

static void SFX_Hurt(void)
{
  Buzzer_Tone(440, 20);
  Buzzer_Tone(330, 30);
  Buzzer_Tone(220, 40);
}

static void SFX_GameOver(void)
{
  Buzzer_Tone(523, 100);
  Buzzer_Tone(392, 100);
  Buzzer_Tone(330, 100);
  Buzzer_Tone(440, 80);
  Buzzer_Tone(494, 80);
  Buzzer_Tone(440, 80);
  Buzzer_Tone(415, 80);
  Buzzer_Tone(466, 80);
  Buzzer_Tone(415, 80);
  Buzzer_Tone(392, 150);
}

static void SFX_StageClear(void)
{
  Buzzer_Tone(392, 80);
  Buzzer_Tone(523, 80);
  Buzzer_Tone(659, 80);
  Buzzer_Tone(784, 80);
  Buzzer_Tone(1047, 80);
  Buzzer_Tone(1319, 80);
  Buzzer_Tone(1568, 200);
  Buzzer_Tone(1319, 300);
}

/* ====================================================================
 * 游戏数据结构与多关卡引擎
 * ==================================================================== */
typedef enum {
  STATE_TITLE,
  STATE_STAGE_INTRO,
  STATE_PLAYING,
  STATE_GAMEOVER,
  STATE_STAGE_CLEAR,
  STATE_GAME_WON
} GameState_t;

static GameState_t game_state = STATE_TITLE;

#define GROUND_Y 52
#define TOTAL_STAGES 3

// 玩家马里奥属性
static int16_t mario_x = 20;
static int16_t mario_y = 37;
static int16_t mario_vy = 0;
static uint8_t on_ground = 1;
static uint8_t facing_left = 0;
static uint8_t anim_frame = 0;
static uint8_t anim_counter = 0;
static uint8_t invulnerable_timer = 0;

static uint32_t score = 0;
static uint8_t coins = 0;
static int8_t lives = 3;
static int16_t camera_x = 0;
static uint8_t current_stage = 1;
static uint8_t stage_intro_timer = 0;

// 地形与物件数据结构
#define MAX_PIPES   4
#define MAX_BLOCKS  8
#define MAX_GOOMBAS 5
#define MAX_PITS    2

typedef struct {
  int16_t x_start;
  int16_t x_end;
} Pit_t;

typedef struct {
  int16_t x;
  int16_t h;
} Pipe_t;

typedef struct {
  int16_t x;
  int16_t y;
  uint8_t has_coin;
  int8_t bump_dy;
} QBlock_t;

typedef struct {
  int16_t x;
  int16_t y;
  int8_t vx;
  uint8_t alive;
  uint8_t squashed_timer;
} Goomba_t;

typedef struct {
  const char *world_str;  // "WORLD 1-1", "WORLD 1-2", "WORLD 1-3"
  uint8_t theme;          // 0: 草原平原, 1: 地下洞穴深坑, 2: 库巴熔岩城堡
  int16_t flag_x;         // 终点旗杆/城堡线
  uint8_t num_pipes;
  Pipe_t pipes[MAX_PIPES];
  uint8_t num_blocks;
  QBlock_t blocks[MAX_BLOCKS];
  uint8_t num_goombas;
  Goomba_t goombas[MAX_GOOMBAS];
  uint8_t num_pits;
  Pit_t pits[MAX_PITS];
} StageData_t;

static const StageData_t stage_presets[TOTAL_STAGES] = {
  // ----------------------------------------------------
  // Stage 1: WORLD 1-1 (经典平原 - 适合上手熟悉手柄)
  // ----------------------------------------------------
  {
    .world_str = "WORLD 1-1",
    .theme = 0,
    .flag_x = 520,
    .num_pipes = 3,
    .pipes = {
      {130, 16},
      {260, 22},
      {380, 16}
    },
    .num_blocks = 5,
    .blocks = {
      {60, 22, 1, 0},
      {160, 22, 1, 0},
      {172, 22, 1, 0},
      {290, 22, 1, 0},
      {410, 22, 1, 0}
    },
    .num_goombas = 3,
    .goombas = {
      {180, GROUND_Y - 10, -1, 1, 0},
      {300, GROUND_Y - 10, -1, 1, 0},
      {440, GROUND_Y - 10, -1, 1, 0}
    },
    .num_pits = 0,
    .pits = {}
  },

  // ----------------------------------------------------
  // Stage 2: WORLD 1-2 (地下深渊密道 - 考验跳跃与浮空踏板)
  // ----------------------------------------------------
  {
    .world_str = "WORLD 1-2",
    .theme = 1,
    .flag_x = 600,
    .num_pipes = 3,
    .pipes = {
      {120, 18},
      {320, 24},
      {450, 18}
    },
    .num_blocks = 6,
    .blocks = {
      {70, 22, 1, 0},
      {150, 22, 1, 0},
      // 跨越 45px 深坑悬崖的浮空跳板 (Pit在 235~280)
      {242, 26, 1, 0},
      {262, 26, 1, 0},
      {360, 22, 1, 0},
      {480, 22, 1, 0}
    },
    .num_goombas = 4,
    .goombas = {
      {160, GROUND_Y - 10, -1, 1, 0},
      {200, GROUND_Y - 10, -1, 1, 0},
      {370, GROUND_Y - 10, -1, 1, 0},
      {510, GROUND_Y - 10, -1, 1, 0}
    },
    .num_pits = 1,
    .pits = {
      {235, 280} // 45像素坠亡深渊！
    }
  },

  // ----------------------------------------------------
  // Stage 3: WORLD 1-3 (库巴熔岩城堡 - 终极大决战)
  // ----------------------------------------------------
  {
    .world_str = "WORLD 1-3",
    .theme = 2,
    .flag_x = 700,
    .num_pipes = 2,
    .pipes = {
      {130, 20},
      {530, 26}
    },
    .num_blocks = 8,
    .blocks = {
      {70, 22, 1, 0},
      // 熔岩深渊 1 上方的立体踏板 (Pit 1 在 190~235)
      {196, 26, 1, 0},
      {216, 18, 1, 0},
      // 中间密集交战区
      {290, 22, 1, 0},
      {330, 22, 1, 0},
      {370, 22, 1, 0},
      // 熔岩深渊 2 上方的浮空踏板 (Pit 2 在 430~475)
      {436, 26, 1, 0},
      {456, 26, 1, 0}
    },
    .num_goombas = 5,
    .goombas = {
      {150, GROUND_Y - 10, -1, 1, 0},
      {260, GROUND_Y - 10, -2, 1, 0}, // 高速板栗仔 (vx = -2)
      {310, GROUND_Y - 10, 2, 1, 0},  // 反向追击巡逻 (vx = 2)
      {390, GROUND_Y - 10, -2, 1, 0},
      {590, GROUND_Y - 10, -2, 1, 0}
    },
    .num_pits = 2,
    .pits = {
      {190, 235}, // 熔岩深渊 1
      {430, 475}  // 熔岩深渊 2
    }
  }
};

static StageData_t active_stage;

static void Load_Stage(uint8_t stage_num)
{
  if (stage_num < 1 || stage_num > TOTAL_STAGES)
    stage_num = 1;
  current_stage = stage_num;
  memcpy(&active_stage, &stage_presets[current_stage - 1], sizeof(StageData_t));

  mario_x = 20;
  mario_y = GROUND_Y - 15;
  mario_vy = 0;
  on_ground = 1;
  camera_x = 0;
  invulnerable_timer = 0;

  Telemetry_Printf("[STAGE] >>> Loaded %s (Theme:%d, Flag:X=%d, Pits:%d) <<<\r\n",
                   active_stage.world_str, active_stage.theme, active_stage.flag_x, active_stage.num_pits);
}

static void Reset_Player_Position(void)
{
  int16_t spawn_x = (camera_x > 20) ? (camera_x + 10) : 20;
  for (int p = 0; p < active_stage.num_pits; p++)
  {
    if (spawn_x >= active_stage.pits[p].x_start - 5 && spawn_x <= active_stage.pits[p].x_end + 5)
    {
      spawn_x = active_stage.pits[p].x_start - 12;
      break;
    }
  }
  if (spawn_x < 20) spawn_x = 20;
  mario_x = spawn_x;
  mario_y = GROUND_Y - 15;
  mario_vy = 0;
  on_ground = 1;
  invulnerable_timer = 50; // 复活闪烁 50 帧
}

/* ====================================================================
 * 按键读取接口 (按下为 0，松开为 1)
 * ==================================================================== */
static inline uint8_t Read_Key_Jump(void)   { return !HAL_GPIO_ReadPin(KEY_JUMP_PORT, KEY_JUMP_PIN); }
static inline uint8_t Read_Key_Left(void)   { return !HAL_GPIO_ReadPin(KEY_LEFT_PORT, KEY_LEFT_PIN); }
static inline uint8_t Read_Key_Right(void)  { return !HAL_GPIO_ReadPin(KEY_RIGHT_PORT, KEY_RIGHT_PIN); }
static inline uint8_t Read_Key_Action(void) { return !HAL_GPIO_ReadPin(KEY_ACTION_PORT, KEY_ACTION_PIN); }

void Mario_Game_Init(void)
{
  // 0. 初始化双通道串口诊断遥测 (USART1 PA9 + USART3 PB10 @ 115200 8N1)
  Telemetry_Init();

  // 1. 初始化 PS2 无线手柄接收器 (PA6=DAT, PA3=CMD, PA4=CS, PA5=CLK)
  PS2_Init();
  Telemetry_Printf("[PS2] Wireless Receiver initialized on PA3/PA4/PA5/PA6.\r\n");

  // 1. 初始化按键引脚 (上拉输入)
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  // PA0: JUMP (按下接地)
  GPIO_InitStruct.Pin = KEY_JUMP_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KEY_JUMP_PORT, &GPIO_InitStruct);

  // PB12: LEFT, PB13: RIGHT, PB9: ACTION (按下接地)
  GPIO_InitStruct.Pin = KEY_LEFT_PIN | KEY_RIGHT_PIN | KEY_ACTION_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // 2. 初始化蜂鸣器引脚 PA7
  GPIO_InitStruct.Pin = BUZZER_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(BUZZER_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);

  // 3. 初始化板载 LED
  GPIO_InitStruct.Pin = LED_READY_PIN | LED_BEAT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(LED_PORT, LED_READY_PIN, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_PORT, LED_BEAT_PIN, GPIO_PIN_RESET);

  // 4. 初始化 OLED 屏幕 (自动扫描 I2C 地址并发送双重芯片兼容初始化)
  uint8_t status = OLED_ScanAndInit();

  if (status == 0)
  {
    Telemetry_Printf("[OLED] ERROR: Screen not responding on I2C bus! Check PB6(SDA), PB7(SCL).\r\n");
    // I2C 未收到屏幕 ACK 响应：发出一声 600ms 低沉警报音 (200Hz)
    Buzzer_Tone(200, 600);
  }
  else
  {
    Telemetry_Printf("[OLED] SUCCESS: OLED found at 0x%02X! Screen online.\r\n", oled_addr);
    // 成功连通屏幕！发出开机双金币音
    SFX_Coin();

    // 绘制 0.8 秒开机自检画面
    OLED_Clear();
    OLED_DrawRect(0, 0, 128, 64, 1);
    OLED_PrintStr(16, 16, "MARIO OLED TEST");
    OLED_PrintStr(16, 32, "I2C ADDR: 0x");
    if (oled_addr == 0x78) OLED_PrintStr(88, 32, "78");
    else if (oled_addr == 0x7A) OLED_PrintStr(88, 32, "7A");
    else OLED_PrintStr(88, 32, "OK");
    OLED_PrintStr(16, 48, "STATUS: READY!");
    OLED_UpdateScreen();
    HAL_Delay(800);
  }

  // 5. 载入第 1 关初始数据
  Load_Stage(1);
}

/* 绘制经典水管 */
static void Draw_Pipe(int16_t screen_x, int16_t h)
{
  if (screen_x < -20 || screen_x > 130) return;
  int16_t top_y = GROUND_Y - h;
  // 管盖 (16x6)
  OLED_DrawRect(screen_x - 1, top_y, 16, 6, 1);
  OLED_FillRect(screen_x, top_y + 1, 14, 4, 1);
  // 管身 (14 x (h - 6))
  OLED_DrawRect(screen_x, top_y + 6, 14, h - 6, 1);
  // 管身纵向高光线
  OLED_DrawLine(screen_x + 3, top_y + 6, screen_x + 3, GROUND_Y - 1, 1);
}

/* 检查指定世界坐标是否处于悬崖深坑中 */
static inline uint8_t Is_In_Pit(int16_t wx)
{
  for (int p = 0; p < active_stage.num_pits; p++)
  {
    if (wx >= active_stage.pits[p].x_start && wx <= active_stage.pits[p].x_end)
      return 1;
  }
  return 0;
}

/* 绘制经典地面砖块与深渊熔岩 */
static void Draw_Ground(int16_t cam_x)
{
  static uint8_t lava_anim = 0;
  lava_anim++;

  for (int16_t sx = 0; sx < 128; sx++)
  {
    int16_t wx = cam_x + sx;
    if (Is_In_Pit(wx))
    {
      if (active_stage.theme == 2)
      {
        if (((sx + (lava_anim / 2)) % 4) == 0)
        {
          OLED_DrawPixel(sx, 61, 1);
        }
        OLED_DrawLine(sx, 62, sx, 63, 1);
      }
      continue;
    }

    OLED_DrawPixel(sx, GROUND_Y, 1);
    OLED_DrawPixel(sx, GROUND_Y + 6, 1);

    if ((wx % 16) == 0)
    {
      OLED_DrawLine(sx, GROUND_Y, sx, 63, 1);
    }
    else if ((wx % 16) == 8)
    {
      OLED_DrawLine(sx, GROUND_Y + 6, sx, 63, 1);
    }
  }
}

void Mario_Game_Update(void)
{
  uint32_t t_frame_start = HAL_GetTick();

  // 1. 读取 PS2 无线手柄数据与按键
  uint8_t ps2_mode = PS2_ReadData();
  uint8_t ps2_left = 0, ps2_right = 0, ps2_jump = 0, ps2_action = 0, ps2_start = 0;
  if (ps2_mode != PSB_LOSE)
  {
    uint8_t lx = PS2_GetAnalogData(PSS_LX);
    ps2_left   = PS2_GetKeyState(PSB_PAD_LEFT)  || (ps2_mode == PSB_REDLIGHT_MODE && lx < 64);
    ps2_right  = PS2_GetKeyState(PSB_PAD_RIGHT) || (ps2_mode == PSB_REDLIGHT_MODE && lx > 192);
    ps2_jump   = PS2_GetKeyState(PSB_CROSS)     || PS2_GetKeyState(PSB_PAD_UP);
    ps2_action = PS2_GetKeyState(PSB_SQUARE)    || PS2_GetKeyState(PSB_CIRCLE) || PS2_GetKeyState(PSB_R1);
    ps2_start  = PS2_GetKeyState(PSB_START)     || PS2_GetKeyState(PSB_SELECT);
  }

  // 2. 融合物理按键与手柄信号 (双控并集，手柄按键与面包板按键同时有效)
  uint8_t key_jump   = Read_Key_Jump()   || ps2_jump;
  uint8_t key_left   = Read_Key_Left()   || ps2_left;
  uint8_t key_right  = Read_Key_Right()  || ps2_right;
  uint8_t key_action = Read_Key_Action() || ps2_action;

  static uint8_t last_jump_key = 0;
  static uint8_t last_left_key = 0;
  static uint8_t last_right_key = 0;
  static uint8_t last_action_key = 0;
  static uint8_t last_ps2_mode = 0xFF;

  // 手柄连接模式变更即时播报
  if (ps2_mode != last_ps2_mode)
  {
    if (ps2_mode == PSB_REDLIGHT_MODE)
      Telemetry_Printf("[PS2] >>> Controller Paired: RED LIGHT (Analog Stick Active) <<<\r\n");
    else if (ps2_mode == PSB_GREENLIGHT_MODE)
      Telemetry_Printf("[PS2] >>> Controller Paired: GREEN LIGHT (Digital Mode) <<<\r\n");
    else
      Telemetry_Printf("[PS2] Controller Searching / Disconnected...\r\n");
    last_ps2_mode = ps2_mode;
  }

  // 按键边沿检测 (按下的一瞬间)
  uint8_t jump_pressed = (key_jump && !last_jump_key);
  uint8_t action_pressed = (key_action && !last_action_key);

  // 串口实时回传按键动作 (防抖电平变化即时推流)
  if (key_jump != last_jump_key)     { Telemetry_LogKey("JUMP", key_jump); }
  if (key_left != last_left_key)     { Telemetry_LogKey("LEFT", key_left); }
  if (key_right != last_right_key)   { Telemetry_LogKey("RIGHT", key_right); }
  if (key_action != last_action_key) { Telemetry_LogKey("ACT", key_action); }

  last_jump_key = key_jump;
  last_left_key = key_left;
  last_right_key = key_right;
  last_action_key = key_action;

  // 心跳指示灯翻转 (PB1)
  static uint8_t beat_cnt = 0;
  if (++beat_cnt >= 18)
  {
    beat_cnt = 0;
    HAL_GPIO_TogglePin(LED_PORT, LED_BEAT_PIN);
  }

  // ==================== 状态机分支 ====================
  if (game_state == STATE_TITLE)
  {
    OLED_Clear();
    OLED_PrintStr(28, 4, "SUPER MARIO");
    OLED_DrawLine(20, 14, 108, 14, 1);

    OLED_DrawBitmap(20, 26, bmp_mario_stand, 12, 15, 0);
    OLED_DrawBitmap(58, 28, bmp_qblock, 10, 10, 0);
    OLED_DrawBitmap(96, 31, bmp_goomba1, 12, 10, 0);

    static uint8_t blink = 0;
    if ((blink++ / 8) % 2 == 0)
    {
      OLED_PrintStr(16, 48, "- PRESS JUMP -");
    }

    Draw_Ground(0);

    if (jump_pressed || action_pressed || ps2_start)
    {
      score = 0;
      coins = 0;
      lives = 3;
      Load_Stage(1);
      stage_intro_timer = 32;
      game_state = STATE_STAGE_INTRO;
      Telemetry_LogEvent("Game Started! State -> STAGE_INTRO");
      SFX_Jump();
    }
  }
  else if (game_state == STATE_STAGE_INTRO)
  {
    OLED_Clear();
    OLED_PrintStr(32, 16, active_stage.world_str);
    OLED_DrawBitmap(44, 32, bmp_mario_stand, 12, 15, 0);
    OLED_PrintChar(64, 36, 'x');
    OLED_PrintNum(76, 36, lives, 2);

    if (stage_intro_timer > 0)
    {
      stage_intro_timer--;
      if (stage_intro_timer == 0)
      {
        game_state = STATE_PLAYING;
      }
    }
  }
  else if (game_state == STATE_GAMEOVER)
  {
    OLED_Clear();
    OLED_PrintStr(34, 10, "GAME OVER");
    OLED_PrintStr(24, 24, "WORLD:");
    OLED_PrintStr(66, 24, active_stage.world_str + 6);
    OLED_PrintStr(24, 38, "SCORE:");
    OLED_PrintNum(68, 38, score, 5);

    static uint8_t blink2 = 0;
    if ((blink2++ / 10) % 2 == 0)
    {
      OLED_PrintStr(16, 52, "- PRESS REPLAY -");
    }

    if (jump_pressed || action_pressed || ps2_start)
    {
      game_state = STATE_TITLE;
      Telemetry_LogEvent("Restart Pressed -> TITLE");
    }
  }
  else if (game_state == STATE_STAGE_CLEAR)
  {
    OLED_Clear();
    OLED_PrintStr(20, 8, "STAGE CLEAR!");
    OLED_PrintStr(24, 22, active_stage.world_str);
    OLED_PrintStr(88, 22, "PASS");
    OLED_PrintStr(24, 36, "SCORE:");
    OLED_PrintNum(68, 36, score, 5);

    static uint8_t blink3 = 0;
    if ((blink3++ / 10) % 2 == 0)
    {
      OLED_PrintStr(16, 52, "- PRESS JUMP -");
    }

    if (jump_pressed || action_pressed || ps2_start)
    {
      current_stage++;
      Load_Stage(current_stage);
      stage_intro_timer = 32;
      game_state = STATE_STAGE_INTRO;
      Telemetry_Printf("[STAGE] Advancing to Stage %d\r\n", current_stage);
      SFX_Jump();
    }
  }
  else if (game_state == STATE_GAME_WON)
  {
    OLED_Clear();
    OLED_PrintStr(14, 8, "CONGRATULATIONS!");
    OLED_PrintStr(10, 22, "PEACH IS RESCUED!");
    OLED_PrintStr(18, 36, "ALL 3 WORLDS WON");
    OLED_PrintStr(24, 48, "SCORE:");
    OLED_PrintNum(68, 48, score, 5);

    if (jump_pressed || action_pressed || ps2_start)
    {
      game_state = STATE_TITLE;
      Telemetry_LogEvent("Victory -> TITLE");
    }
  }
  else if (game_state == STATE_PLAYING)
  {
    uint8_t speed = key_action ? 3 : 2;

    // 左右移动控制
    if (key_left)
    {
      mario_x -= speed;
      if (mario_x < camera_x) mario_x = camera_x;
      facing_left = 1;
      anim_counter++;
      if (anim_counter >= 3) { anim_counter = 0; anim_frame ^= 1; }
    }
    else if (key_right)
    {
      mario_x += speed;
      facing_left = 0;
      anim_counter++;
      if (anim_counter >= 3) { anim_counter = 0; anim_frame ^= 1; }
    }
    else
    {
      anim_frame = 0;
    }

    // 跳跃控制
    if (jump_pressed && on_ground)
    {
      mario_vy = -7;
      on_ground = 0;
      Telemetry_LogEvent("Mario Jumped!");
      SFX_Jump();
    }

    // 重力下落
    mario_vy += 1;
    if (mario_vy > 6) mario_vy = 6;
    mario_y += mario_vy;

    // 深渊深坑判断
    uint8_t over_pit = 0;
    for (int p = 0; p < active_stage.num_pits; p++)
    {
      if (mario_x + 3 >= active_stage.pits[p].x_start && mario_x + 9 <= active_stage.pits[p].x_end)
      {
        over_pit = 1;
        break;
      }
    }

    on_ground = 0;

    // 地面碰撞检测 (非悬崖深坑)
    if (!over_pit && mario_y >= GROUND_Y - 15)
    {
      mario_y = GROUND_Y - 15;
      mario_vy = 0;
      on_ground = 1;
    }

    // 水管碰撞检测
    for (int i = 0; i < active_stage.num_pipes; i++)
    {
      int16_t px = active_stage.pipes[i].x;
      int16_t py = GROUND_Y - active_stage.pipes[i].h;
      if (mario_x + 10 >= px && mario_x <= px + 14)
      {
        if (mario_y + 15 >= py && (mario_y + 15 - mario_vy) <= py + 3 && mario_vy >= 0)
        {
          mario_y = py - 15;
          mario_vy = 0;
          on_ground = 1;
        }
        else if (mario_y + 15 > py)
        {
          if (mario_x < px) mario_x = px - 11;
          else if (mario_x > px) mario_x = px + 15;
        }
      }
    }

    // 问号/踏板碰撞检测
    for (int i = 0; i < active_stage.num_blocks; i++)
    {
      int16_t bx = active_stage.blocks[i].x;
      int16_t by = active_stage.blocks[i].y;
      if (mario_x + 9 >= bx && mario_x <= bx + 8)
      {
        if (mario_y <= by + 10 && (mario_y - mario_vy) >= by + 4 && mario_vy <= 0)
        {
          mario_y = by + 11;
          mario_vy = 1; // 撞头反弹
          if (active_stage.blocks[i].has_coin)
          {
            active_stage.blocks[i].has_coin = 0;
            active_stage.blocks[i].bump_dy = -3;
            score += 100;
            coins += 1;
            Telemetry_Printf("[EVENT] Block Hit -> +1 Coin, +100 Pts! Total Coins:%d\r\n", coins);
            SFX_Coin();
          }
          else
          {
            active_stage.blocks[i].bump_dy = -2;
            SFX_Bump();
          }
        }
        else if (mario_y + 15 >= by && (mario_y + 15 - mario_vy) <= by + 6 && mario_vy >= 0)
        {
          mario_y = by - 15;
          mario_vy = 0;
          on_ground = 1;
        }
      }

      if (active_stage.blocks[i].bump_dy < 0) active_stage.blocks[i].bump_dy++;
    }

    // 掉入深渊/熔岩检测
    if (mario_y >= 63)
    {
      lives--;
      Telemetry_Printf("[EVENT] Mario fell into abyss! Lives remaining: %d\r\n", lives);
      SFX_Hurt();
      if (lives <= 0)
      {
        Telemetry_LogEvent("Game Over by abyss fall!");
        SFX_GameOver();
        game_state = STATE_GAMEOVER;
      }
      else
      {
        Reset_Player_Position();
      }
    }

    // 板栗仔怪兽巡逻与碰撞
    for (int i = 0; i < active_stage.num_goombas; i++)
    {
      if (!active_stage.goombas[i].alive)
      {
        if (active_stage.goombas[i].squashed_timer > 0) active_stage.goombas[i].squashed_timer--;
        continue;
      }

      active_stage.goombas[i].x += active_stage.goombas[i].vx;
      for (int p = 0; p < active_stage.num_pipes; p++)
      {
        if (active_stage.goombas[i].x >= active_stage.pipes[p].x - 12 &&
            active_stage.goombas[i].x <= active_stage.pipes[p].x + 14)
        {
          active_stage.goombas[i].vx = -active_stage.goombas[i].vx;
        }
      }
      for (int pt = 0; pt < active_stage.num_pits; pt++)
      {
        if (active_stage.goombas[i].x >= active_stage.pits[pt].x_start - 8 &&
            active_stage.goombas[i].x <= active_stage.pits[pt].x_end + 8)
        {
          active_stage.goombas[i].vx = -active_stage.goombas[i].vx;
        }
      }

      // 碰撞检测
      if (mario_x + 10 >= active_stage.goombas[i].x && mario_x <= active_stage.goombas[i].x + 10)
      {
        if (mario_y + 15 >= active_stage.goombas[i].y && (mario_y + 15 - mario_vy) <= active_stage.goombas[i].y + 4 && mario_vy > 0)
        {
          active_stage.goombas[i].alive = 0;
          active_stage.goombas[i].squashed_timer = 15;
          mario_vy = -5;
          score += 200;
          Telemetry_LogEvent("Goomba Stomped! +200 Pts");
          SFX_Stomp();
        }
        else if (mario_y + 15 > active_stage.goombas[i].y + 2 && invulnerable_timer == 0)
        {
          lives--;
          invulnerable_timer = 40;
          Telemetry_Printf("[EVENT] Mario Hurt! Lives remaining: %d\r\n", lives);
          SFX_Hurt();
          mario_vy = -4;
          if (lives <= 0)
          {
            Telemetry_LogEvent("Game Over!");
            SFX_GameOver();
            game_state = STATE_GAMEOVER;
          }
        }
      }
    }

    if (invulnerable_timer > 0) invulnerable_timer--;

    // 到达关卡终点检测
    if (mario_x >= active_stage.flag_x && game_state == STATE_PLAYING)
    {
      score += 1000;
      SFX_StageClear();
      if (current_stage < TOTAL_STAGES)
      {
        Telemetry_Printf("[EVENT] Stage %d Cleared! Score: %lu\r\n", current_stage, score);
        game_state = STATE_STAGE_CLEAR;
      }
      else
      {
        Telemetry_Printf("[EVENT] ALL STAGES CLEARED! Final Score: %lu\r\n", score);
        game_state = STATE_GAME_WON;
      }
    }

    // 摄像机镜头跟随马里奥
    if (mario_x - camera_x > 56)
    {
      camera_x = mario_x - 56;
    }

    // 渲染本帧游戏画面
    OLED_Clear();

    // 1. 主题特色背景
    if (active_stage.theme == 0)
    {
      // 草原主题: 白云与灌木
      OLED_DrawBitmap(30 - (camera_x / 3) % 150, 8, bmp_cloud, 16, 10, 0);
      OLED_DrawBitmap(110 - (camera_x / 3) % 150, 12, bmp_cloud, 16, 10, 0);
      OLED_DrawBitmap(80 - camera_x, GROUND_Y - 8, bmp_bush, 16, 8, 0);
      OLED_DrawBitmap(200 - camera_x, GROUND_Y - 8, bmp_bush, 16, 8, 0);
      OLED_DrawBitmap(330 - camera_x, GROUND_Y - 8, bmp_bush, 16, 8, 0);
    }
    else if (active_stage.theme == 1)
    {
      // 地下洞穴主题: 经典深邃纯黑洞穴背景 (保留顶部 HUD 黄色状态栏纯净无干扰)
    }
    else if (active_stage.theme == 2)
    {
      // 熔岩城堡主题: 黑暗城堡背景，重点突出底部翻滚岩浆与终点黄金战斧
    }

    // 2. 地面与水管
    Draw_Ground(camera_x);
    for (int i = 0; i < active_stage.num_pipes; i++)
    {
      Draw_Pipe(active_stage.pipes[i].x - camera_x, active_stage.pipes[i].h);
    }

    // 3. 问号砖块与踏板
    for (int i = 0; i < active_stage.num_blocks; i++)
    {
      int16_t sx = active_stage.blocks[i].x - camera_x;
      int16_t sy = active_stage.blocks[i].y + active_stage.blocks[i].bump_dy;
      if (sx >= -12 && sx <= 128)
      {
        if (active_stage.blocks[i].has_coin)
          OLED_DrawBitmap(sx, sy, bmp_qblock, 10, 10, 0);
        else
          OLED_DrawBitmap(sx, sy, bmp_eblock, 10, 10, 0);
      }
    }

    // 4. 板栗仔敌人
    static uint8_t g_anim = 0;
    g_anim++;
    for (int i = 0; i < active_stage.num_goombas; i++)
    {
      int16_t gx = active_stage.goombas[i].x - camera_x;
      if (gx >= -14 && gx <= 128)
      {
        if (active_stage.goombas[i].alive)
        {
          const uint8_t *g_bmp = ((g_anim / 6) % 2 == 0) ? bmp_goomba1 : bmp_goomba2;
          OLED_DrawBitmap(gx, active_stage.goombas[i].y, g_bmp, 12, 10, 0);
        }
        else if (active_stage.goombas[i].squashed_timer > 0)
        {
          OLED_DrawBitmap(gx, GROUND_Y - 5, bmp_goomba_flat, 12, 5, 0);
        }
      }
    }

    // 5. 终点 (平原/洞穴为旗杆，城堡为大门与黄金战斧)
    int16_t fx = active_stage.flag_x - camera_x;
    if (active_stage.theme < 2)
    {
      if (fx >= -10 && fx <= 128)
      {
        OLED_DrawLine(fx, 10, fx, GROUND_Y, 1);
        OLED_FillRect(fx - 2, 8, 5, 3, 1);     // 杆顶金球
        OLED_DrawLine(fx - 8, 12, fx, 16, 1);   // 旗帜
        OLED_DrawLine(fx - 8, 20, fx, 16, 1);
        OLED_DrawLine(fx - 8, 12, fx - 8, 20, 1);
      }
    }
    else
    {
      if (fx >= -20 && fx <= 128)
      {
        // 城堡大门拱门
        OLED_DrawRect(fx, 18, 22, GROUND_Y - 18, 1);
        OLED_DrawLine(fx + 3, 15, fx + 18, 15, 1);
        OLED_DrawLine(fx + 6, 12, fx + 15, 12, 1);
        // 悬挂在门前的黄金战斧 (Golden Axe)
        OLED_DrawLine(fx - 6, 30, fx - 6, 44, 1);
        OLED_FillRect(fx - 10, 28, 5, 6, 1);
      }
    }

    // 6. 马里奥主角 (受伤无敌时交替闪烁)
    if (invulnerable_timer == 0 || (invulnerable_timer % 4 < 2))
    {
      int16_t screen_mario_x = mario_x - camera_x;
      const uint8_t *m_bmp = bmp_mario_stand;
      if (!on_ground)
        m_bmp = bmp_mario_jump;
      else if (anim_frame)
        m_bmp = bmp_mario_run;

      OLED_DrawBitmap(screen_mario_x, mario_y, m_bmp, 12, 15, facing_left);
    }

    // 7. 顶部 HUD 状态栏
    OLED_PrintStr(0, 0, "M");
    OLED_PrintNum(8, 0, score, 5);

    OLED_DrawBitmap(42, 0, bmp_coin, 8, 8, 0);
    OLED_PrintChar(51, 0, 'x');
    OLED_PrintNum(57, 0, coins, 2);

    OLED_PrintStr(73, 0, "W1-");
    OLED_PrintNum(91, 0, current_stage, 1);

    OLED_PrintStr(103, 0, "HP");
    OLED_PrintNum(116, 0, lives, 1);
  }

  // 计算逻辑与渲染准备耗时
  uint32_t t_logic = HAL_GetTick() - t_frame_start;

  // 统一物理刷新屏幕
  uint32_t t_oled_start = HAL_GetTick();
  OLED_UpdateScreen();
  uint32_t t_oled = HAL_GetTick() - t_oled_start;

  // 动态锁定 35 FPS (目标每帧 28ms，平滑丝滑，消除卡顿与抖动)
  uint32_t t_busy = HAL_GetTick() - t_frame_start;
  if (t_busy < 28)
  {
    HAL_Delay(28 - t_busy);
  }
  uint32_t t_total = HAL_GetTick() - t_frame_start;

  // 每 30 帧 (约 1 秒) 向串口回传一次完整诊断遥测数据
  static uint32_t frame_count = 0;
  static uint32_t last_report_tick = 0;
  frame_count++;
  if (frame_count >= 30)
  {
    uint32_t now = HAL_GetTick();
    uint32_t dt = now - last_report_tick;
    last_report_tick = now;
    uint32_t current_fps = (dt > 0) ? ((frame_count * 1000) / dt) : 35;
    frame_count = 0;

    const char *st_str = (game_state == STATE_TITLE) ? "TITLE" :
                         (game_state == STATE_STAGE_INTRO) ? "INTRO" :
                         (game_state == STATE_PLAYING) ? "PLAY" :
                         (game_state == STATE_GAMEOVER) ? "GAMEOVER" :
                         (game_state == STATE_STAGE_CLEAR) ? "CLEAR" : "WON";

    const char *ps2_str = (ps2_mode == PSB_REDLIGHT_MODE) ? "RED" :
                         (ps2_mode == PSB_GREENLIGHT_MODE) ? "GREEN" : "OFFLINE";

    Telemetry_LogFrame(current_fps, t_total, t_oled, t_logic,
                       mario_x, mario_y, camera_x,
                       key_jump, key_left, key_right, key_action,
                       st_str, ps2_str);
  }
}
