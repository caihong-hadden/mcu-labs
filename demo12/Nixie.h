/**
 * @file Nixie.h
 * @brief 蓝桥杯板载共阳数码管驱动模块头文件
 */

#ifndef __NIXIE_H__
#define __NIXIE_H__

/**
 * @brief 数码管单物理位动态刷新函数
 * @param loc 数码管位置 (1 ~ 8, 对应第 1 ~ 8 位数码管)
 * @param Num 显示字符编码 (0~9 显示对应数字, 10 为消隐全灭)
 */
void Nixie(unsigned char loc, unsigned char Num);

#endif
