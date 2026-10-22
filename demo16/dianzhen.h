/**
 * @file dianzhen.h
 * @brief 8x8 LED 点阵屏驱动模块头文件
 */

#ifndef __DIANZHEN_H__
#define __DIANZHEN_H__

/**
 * @brief 在 8x8 点阵屏上点亮指定坐标的点 (X, Y)
 * @param X 行坐标 (0 ~ 7)
 * @param Y 列坐标 (0 ~ 7)
 */
void dianzhen(unsigned char X, unsigned char Y);

#endif
