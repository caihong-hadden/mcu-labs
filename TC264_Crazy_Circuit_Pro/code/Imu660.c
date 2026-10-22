/*
 * Imu660.c
 *
 *  Created on: 2026年1月18日
 *      Author: Administrator
 */

#include "zf_common_headfile.h"
int gyro_flag = 0;          // 陀螺仪就绪标志
int gyro_count = 0;         // 陀螺仪计数
long gyro_angle = 0;        // 陀螺仪角度
POSTURE post = {0,0.0,0.0,0.0}; // 姿态数据结构体

void gyro_init()
{
    while(1)
    {
        if(imu660rb_init())
                printf("\r\nIMU660RB init error.");     // IMU660RB 初始化失败
        else
                break;
    }
}

void Gyro_Get(void)       // 获取角速度
{
    imu660rb_get_gyro();
    post.gyro_yaw = imu660rb_gyro_z;
    if(post.gyro_yaw<=5&&post.gyro_yaw>=-5)post.gyro_yaw =0;                      // 死区滤波，小于5的值置零
}


void Gyro_Get_Angle(void) // 获取角度
{
    // post.yaw_angle为小车当前角度，逻辑为：小车正前方为0°，顺时针为正，逆时针为负
    post.yaw_angle += imu660rb_gyro_transition(post.gyro_yaw) * 0.005;             // 获取角速度并乘以时间间隔得到角度变化值
    if(post.yaw_angle>360)          post.yaw_angle = 0;
    else if(post.yaw_angle<0)       post.yaw_angle = 360;

    if(ImageFlag.image_element_angle_flag >= 2 || ImageFlag.image_element_Node_flag >= 2
        || ImageFlag.image_element_branch_flag >= 2 || ImageFlag.image_element_cross_flag >= 2)                                // 等待目标角度已写入后再开始转弯
    {
        post.diff_angle = post.target_angle - post.yaw_angle;                        // 大于0右转

        if(post.diff_angle > 180)           post.diff_angle -= 360;                  // 计算最短角度差（限制在 -180° 至 180° 之间）
        else if(post.diff_angle < -180)     post.diff_angle += 360;

        if(ImageFlag.image_element_angle_flag == 2)ImageFlag.image_element_angle_flag = 3;     // 元素标志位设为3（执行中）
        if(ImageFlag.image_element_Node_flag == 2)ImageFlag.image_element_Node_flag = 3;       // 元素标志位设为3（执行中）
        if(ImageFlag.image_element_branch_flag == 2)ImageFlag.image_element_branch_flag = 3;   // 元素标志位设为3（执行中）
        if(ImageFlag.image_element_cross_flag == 2)ImageFlag.image_element_cross_flag = 3;     // 元素标志位设为3（执行中）
    }
}
