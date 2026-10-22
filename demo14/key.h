/**
 * @file key.h
 * @brief 独立按键模块头文件
 */

#ifndef __KEY_H__
#define __KEY_H__

/**
 * @brief 读取独立按键键码
 * @return 0-无按键，1~4 对应 S7~S4 按键
 */
unsigned char Key(void);

#endif
