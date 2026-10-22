#ifndef __PS2_H
#define __PS2_H

#include "main.h"
#include <stdint.h>

// PS2 状态与模式定义
#define PSB_LOSE             0  // 未连接 / 离线
#define PSB_REDLIGHT_MODE    1  // 红灯模式 (模拟摇杆 + 按键)
#define PSB_GREENLIGHT_MODE  2  // 绿灯模式 (纯按键)

// 16 个按键 ID 编号 (对应 16 位 Handkey 掩码)
#define PSB_SELECT      1
#define PSB_L3          2
#define PSB_R3          3
#define PSB_START       4
#define PSB_PAD_UP      5
#define PSB_PAD_RIGHT   6
#define PSB_PAD_DOWN    7
#define PSB_PAD_LEFT    8
#define PSB_L2          9
#define PSB_R2          10
#define PSB_L1          11
#define PSB_R1          12
#define PSB_TRIANGLE    13  // 绿 / 三角
#define PSB_CIRCLE      14  // 红 / 圆圈
#define PSB_CROSS       15  // 蓝 / 叉号 (马里奥核心跳跃键)
#define PSB_SQUARE      16  // 粉 / 方块 (冲刺/技能键)

// 模拟摇杆通道索引 (Data[5] ~ Data[8])
#define PSS_RX          5   // 右摇杆 X 轴 (0=最左, 128=中位, 255=最右)
#define PSS_RY          6   // 右摇杆 Y 轴 (0=最上, 128=中位, 255=最下)
#define PSS_LX          7   // 左摇杆 X 轴 (0=最左, 128=中位, 255=最右)
#define PSS_LY          8   // 左摇杆 Y 轴 (0=最上, 128=中位, 255=最下)

// 引脚定义：
// DAT (DI)  -> PA6 (输入上拉)
// CMD (DO)  -> PA3 (推挽输出)
// CS  (ATT) -> PA4 (推挽输出)
// CLK (SCK) -> PA5 (推挽输出)
#define PS2_PORT        GPIOA
#define PS2_DI_PIN      GPIO_PIN_6
#define PS2_DO_PIN      GPIO_PIN_3
#define PS2_CS_PIN      GPIO_PIN_4
#define PS2_SCK_PIN     GPIO_PIN_5

// 接口函数
void    PS2_Init(void);
uint8_t PS2_ReadData(void);
uint8_t PS2_GetKeyState(uint8_t key_id);
uint8_t PS2_GetAnalogData(uint8_t stick_id);
uint8_t PS2_GetMode(void);
uint16_t PS2_GetRawKeys(void);

#endif /* __PS2_H */
