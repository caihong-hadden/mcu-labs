/*
 * Imu660.h
 *
 *  Created on: 2026年1月18日
 *      Author: Administrator
 */

#ifndef CODE_IMU660_H_
#define CODE_IMU660_H_

typedef struct
{
    int gyro_yaw;            // 陀螺仪偏航角速度
    double yaw_angle;        // 当前偏航角度（0°~360°）
    double target_angle;     // 目标偏航角度
    double diff_angle;       // 目标角度与当前角度的差值
}POSTURE;                    // 姿态数据结构体


extern int gyro_flag;        // 陀螺仪就绪标志
extern int gyro_count;       // 陀螺仪计数
extern POSTURE post;         // 姿态数据实例

void gyro_init(void);        // 陀螺仪初始化
void Gyro_Get(void);         // 获取角速度
void Gyro_Get_Angle(void);   // 获取并计算角度


#endif /* CODE_IMU660_H_ */
