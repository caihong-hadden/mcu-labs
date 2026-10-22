/**
 * @file Mitrixkey.h
 * @brief 蓝桥杯 4x4 矩阵键盘底层驱动头文件
 */

#ifndef __MITRIXKEY_H__
#define __MITRIXKEY_H__

/**
 * @brief 扫描 4x4 矩阵键盘并返回按下的键码
 * @return 0 表示无按键按下，非零值 (1 ~ 16) 表示对应按键按下
 */
unsigned char MatrixKey(void);

#endif
