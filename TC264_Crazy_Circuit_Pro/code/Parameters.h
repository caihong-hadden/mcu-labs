/*
 * Parameters.h
 *
 *  全局参数初始化模块头文件 — 声明路径预设数据、参数初始化与系统总初始化函数
 *
 *  Created on: 2026年1月18日
 *      Author: Administrator
 */

#ifndef CODE_PARAMETERS_H_
#define CODE_PARAMETERS_H_

#include "zf_common_headfile.h"

extern int Path_Switch;   // 路径切换选择（0=项目一，1=项目二，2=项目三）

void All_Init(void);        // 系统总初始化：依次初始化参数、无线、传感器、执行器等所有外设
void Parameters_Init(void); // 参数初始化：设置PID、视觉、速度等全部运行参数的默认值

#endif /* CODE_PARAMETERS_H_ */
