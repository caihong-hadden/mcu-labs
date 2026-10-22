/*
 * menu.h
 *
 *  Created on: 2026年1月18日
 *      Author: Administrator
 */

#ifndef CODE_MENU_H_
#define CODE_MENU_H_

#include "zf_common_headfile.h"

#define BUZZER_PIN              (P33_10)   // 蜂鸣器引脚
#define LED_1_PIN               (P20_9)    // LED1引脚
#define LED_2_PIN               (P20_8)    // LED2引脚
#define LED_3_PIN               (P21_5)    // LED3引脚

typedef struct
{
    int run_status;      // 运行状态标志
    int end;             // 结束标志
    int protect;         // 保护状态标志
    int element_num;     // 当前元素编号
    int element_run;     // 元素运行状态

}STATUS;

extern STATUS status;          // 全局状态结构体
extern int image_solve_fps;    // 图像解算帧率

void buzzer_on(void);                      // 蜂鸣器鸣响
void buzzer_off(void);                     // 蜂鸣器关闭
void buzzer_init(void);                    // 蜂鸣器初始化
void buzzer_turn(void);                    // 蜂鸣器电平翻转

void image_line_show(void);                // 图像处理结果显示（赛道边界线等）
void battery_voltage_show(void);           // 电池电压显示
void Page_In_Out(void);                    // 菜单进入与退出处理
void menu_0(void);
void menu_1(void);
void menu_2(void);                           // 菜单显示（赛车状态信息）
void menus_solve(void);//具体需要啥，可以根据我的逻辑去添加
void Menu_Logic_Run(void);                 // 菜单逻辑运行（周期性调用）
void LED_init(void);                       // LED指示灯初始化
void LED_on(int LED_number);               // 点亮指定LED
void LED_off(void);                        // 关闭所有LED
void cursor_move(void);
uint8_t key_num_get(void);
void key_handle(void);//获取按键加减状态

#endif /* CODE_MENU_H_ */
