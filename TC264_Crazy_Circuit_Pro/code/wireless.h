/*
 * wireless.h
 *
 *  无线串口调试模块头文件 — 声明Vofa+数据显示与在线调参相关函数
 *
 *  Created on: 2026年1月22日
 *      Author: Administrator
 */

#ifndef CODE_WIRELESS_H_
#define CODE_WIRELESS_H_

#include "zf_common_headfile.h"

extern uint8_t Wireless_init_flag;   // 无线模块初始化标志位，供其他模块查询无线串口是否就绪

void Vofa_Show(void);                // Vofa+上位机数据显示
void Vofa_Adjust_Parameter(void);    // Vofa+上位机在线参数调整

#endif /* CODE_WIRELESS_H_ */
