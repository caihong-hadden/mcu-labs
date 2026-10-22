/**
 * @file Delay.h
 * @brief 软件阻塞延时模块头文件
 * @details 适用于 STC15 系列单片机 (1T 模式) @11.0592MHz
 */

#ifndef __DELAY_H__
#define __DELAY_H__

/**
 * @brief 毫秒级软件阻塞延时
 * @param xms 延时时长 (ms)
 */
void Delay(unsigned int xms);

#endif
