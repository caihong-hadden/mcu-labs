#ifndef __SERVO_H
#define __SERVO_H

#include "main.h"

/* 舵机脉宽边界定义 (标准 RC 舵机 0.5ms ~ 2.5ms @ 50Hz) */
#define SERVO_MIN_PULSE_US  500    // 0 度对应脉宽 (微秒)
#define SERVO_MAX_PULSE_US  2500   // 180 度对应脉宽 (微秒)
#define SERVO_MID_PULSE_US  1500   // 90 度中位脉宽 (微秒)

#define SERVO_MIN_ANGLE     0.0f
#define SERVO_MAX_ANGLE     180.0f

/* 核心 API */
void     Servo_Init(void);
void     Servo_SetAngle(float angle);
void     Servo_SetPulse(uint16_t pulse_us);
float    Servo_GetAngle(void);
uint16_t Servo_GetPulse(void);

/* 智能平滑扫动模式 (SWEEP) 控制 */
void     Servo_SetSweepMode(uint8_t enable);
uint8_t  Servo_GetSweepMode(void);
void     Servo_UpdateSweep(void);

#endif /* __SERVO_H */
