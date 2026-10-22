/*
 * menu.c
 *
 *  Created on: 2026年1月18日
 *      Author: Administrator
 */

#include "zf_common_headfile.h"

uint8_t menu_number = 0; //菜单编号
int image_solve_fps = 0; // 图像解算帧率
STATUS status = {0,0,0,0,0};
int cursor_pos = 0; //当前位置
int last_cursor_pos = 0; //上次位置
int last_menu_cursor_pos = 0; //上次菜单位置
uint8_t data_change_flag = 0; //数据变化标志位
uint8_t data_up_flag = 0; //数据上升标志位
uint8_t data_down_flag = 0; //数据下降标志位
uint8_t key_num = 0; //按键编号
int menu_limit_pos[] = {2, 10, 0}; //各菜单最大光标位置

// 初始化LED指示灯引脚（高电平熄灭）
void LED_init(void)
{
    gpio_init(LED_1_PIN, GPO, GPIO_HIGH, GPO_PUSH_PULL);
    gpio_init(LED_2_PIN, GPO, GPIO_HIGH, GPO_PUSH_PULL);
    gpio_init(LED_3_PIN, GPO, GPIO_HIGH, GPO_PUSH_PULL);
}

// 关闭所有LED指示灯
void LED_off(void)
{
    gpio_set_level(LED_1_PIN, GPIO_HIGH);
    gpio_set_level(LED_2_PIN, GPIO_HIGH);
    gpio_set_level(LED_3_PIN, GPIO_HIGH);
}

// 点亮指定编号的LED（0=LED1, 1=LED2, 2=LED3）
void LED_on(int LED_number)
{
    switch (LED_number)
    {
    case 0:
        LED_off();
        gpio_set_level(LED_1_PIN, GPIO_LOW);
        break;
    case 1:
        LED_off();
        gpio_set_level(LED_2_PIN, GPIO_LOW);
        break;
    case 2:
        LED_off();
        gpio_set_level(LED_3_PIN, GPIO_LOW);
        break;
    default:
        break;
    }
}

// 初始化蜂鸣器引脚（低电平关闭）
void buzzer_init(void)
{
    gpio_init(BUZZER_PIN, GPO, GPIO_LOW, GPO_PUSH_PULL);
}

// 蜂鸣器鸣响
void buzzer_on(void)
{
    gpio_set_level(BUZZER_PIN, GPIO_HIGH);
}

// 蜂鸣器关闭
void buzzer_off(void)
{
    gpio_set_level(BUZZER_PIN, GPIO_LOW);
}

// 蜂鸣器电平翻转（用于产生鸣响效果）
void buzzer_turn(void)
{
    if(gpio_get_level(BUZZER_PIN) == GPIO_HIGH)
    {
        gpio_set_level(BUZZER_PIN, GPIO_LOW);
    }
    else
    {
        gpio_set_level(BUZZER_PIN, GPIO_HIGH);
    }
}

uint8_t key_num_get(void)
{
    uint8_t key_state = 0;

    if(key_get_state(KEY_1) == 1)
    {
        key_state = 1;
    }
    else if(key_get_state(KEY_2) == 1)
    {
        key_state = 2;
    }
    else if (key_get_state(KEY_3) == 1)
    {
        key_state = 3;
    }
    else if(key_get_state(KEY_4) == 1)
    {
        key_state = 4;
    }
    else if(key_get_state(KEY_3) == 2)
    {
        key_state = 5;
    }
    
    return key_state;
}

void key_handle(void)//获取按键加减状态
{
    key_scanner();
    key_num = key_num_get();

    // 编码器+两按键/四按键控制菜单共存方案
    if(data_change_flag == 0)
    {
        if(key_num == 1)       cursor_pos--;
        if(key_num == 2)       cursor_pos++;

        if(current_encoder != 0)
        {
            if(last_cursor_pos != cursor_pos) last_cursor_pos = cursor_pos;
            cursor_pos += current_encoder;

        }
        cursor_pos = (cursor_pos > menu_limit_pos[menu_number]) ? 0 : (cursor_pos < 0) ? menu_limit_pos[menu_number] : cursor_pos;
    }
    else
    {
        data_up_flag = 0;
        data_down_flag = 0;
        if(key_num == 1)       data_up_flag = 1;
        if(key_num == 2)       data_down_flag = 1;
        (current_encoder > 0) ? (data_up_flag = 1) : (current_encoder < 0) ? (data_down_flag = 1) : 0;
    }
    current_encoder = 0;
}

void cursor_move(void)
{
    key_handle();   //按键处理
    
    if(data_change_flag == 0) //如果不是数据修改状态
    {
        //光标移动处理
        ips114_show_string(8*0, 16*last_cursor_pos, "  ");     //清除上次光标
        ips114_show_string(8*0, 16*cursor_pos, "--");          //显示本次光标
        last_cursor_pos = cursor_pos;
    }
    else
    {
        ips114_show_string(8*0, 16*cursor_pos, "->");          //显示本次光标
        cursor_pos = last_cursor_pos; //数据修改状态下禁止光标移动
    }
}

// 显示电池电压、离线距离、前视点和阈值等底层信息
void battery_voltage_show(void)
{
    uint16_t adc_value = adc_mean_filter_convert(ADC0_CH11_A11, 10);
    float battery_voltage = adc_value * 3.3 / 4096 * 11;
    ips114_show_float(0, 16*14, battery_voltage, 2, 1);
    ips114_show_int(8*5, 16*14, ImageStatus.OFFLine_Last, 2);
    ips114_show_int(8*8, 16*14, TowPoint, 2);
    ips114_show_int(8*11, 16*14, threshold_total, 3);
}

/**
 * @brief 图像处理结果显示函数 - 绘制赛道边界线、控制视觉线和补线等显示内容
 *
 * 该函数将图像处理结果绘制到显示屏上，作为一个统一的显示接口，在屏幕底部显示：
 * 1. 赛道边界线：左边界（蓝色）、右边界（粉色）、中心线（红色）
 * 2. 控制视觉线：红色水平线；前视线：紫色水平线
 * 3. 补线：红色垂直线
 *
 * 显示范围：从屏幕第16*8行开始，覆盖60行的图像缓冲区
 */
void image_line_show(void)
{
    int x, y, z;
    uint16_t i;
    
    /* 第一部分：显示赛道边界线和中心线 */
    // 显示范围：从第16*8行到第16*8+60行
    for(i = 0; i <60; i++)
    {
        x = ImageDeal[i].LeftBorder;    // 左边界坐标
        y = ImageDeal[i].RightBorder;   // 右边界坐标
        z = ImageDeal[i].Center;        // 中心线坐标
        
        // 绘制左边界线（蓝色），仅当在有效范围内（1-79）
        if(x >= 0 && x <= 79 )
        {
            ips114_draw_point((uint16_t)x, i+16*10, RGB565_BLUE);
            ips114_draw_point((uint16_t)x+1, i+16*10, RGB565_BLUE);
        }
        // 绘制右边界线（粉色），仅当在有效范围内（1-79）
        if(y >= 0 && y <= 79 )
        {
            ips114_draw_point((uint16_t)y, i+16*10, RGB565_PINK);
            ips114_draw_point((uint16_t)y+1, i+16*10, RGB565_PINK);
        }
        // 绘制中心线（红色），仅当在有效范围内（1-79）
        if(z >= 0 && z <= 79) 
        {
            ips114_draw_point((uint16_t)z, i+16*10, RGB565_RED);
            ips114_draw_point((uint16_t)z+1, i+16*10, RGB565_RED);
        }
    }

    for (i = 0; i < 20; i++)  // 显示传感器中线，用于观察是否偏离赛道
    {
        ips114_draw_point(ImageSensorMid, i+16*10+60, RGB565_BROWN);
        ips114_draw_point(ImageSensorMid+1, i+16*10+60, RGB565_BROWN);
    }
    
    /* 第二部分：显示控制视觉线和前视线 */
    // 显示控制视觉线（红色水平线）和前视线（紫色水平线）
    for(i = 0; i <= 10; i++)
    {
        // 绘制控制视觉线（红色）- 显示图像视野有效边界
        ips114_draw_point(i, ImageStatus.OFFLine_Last + 16*10, RGB565_RED);

        // 绘制前视线（紫色）- 显示偏差控制使用的前视点位置
        ips114_draw_point(i, TowPoint + 16*10 + 10, RGB565_PURPLE);
        ips114_draw_point(i, TowPoint + 16*10 - 10, RGB565_PURPLE);
    }
}

void Page_In_Out(void)
{
    //Page In
    if(key_num == 3 && menu_number != 2)//running页面不需要进行调参，只需要进行发车即可！！！
    {
        if(menu_number != 0)    //如果当前菜单不是主页面
        {
            if(!data_change_flag)   data_change_flag = 1;//进入数据修改状态
            else                    data_change_flag = 0;//退出数据修改状态
        }
        else//如果当前菜单是主页面
        {
            if(cursor_pos == 0)
            {
                Path_Switch++;
                if(Path_Switch > 2) Path_Switch = 0;
                LED_on(Path_Switch);
            }
            else
            {
                last_menu_cursor_pos = cursor_pos;
                menu_number = cursor_pos;
                cursor_pos = 0;
                last_cursor_pos = 0;
                ips114_clear();
            }
        }
    }
    else if(key_num == 3 && menu_number == 2)//running页面需要进行发车
    {
        if(stop_flag)
        {
            race_start_request();
        }
        else
        {
            race_request_stop(RACE_STOP_MANUAL);
            motor.speedTL = 0;
            motor.speedTR = 0;
            motor.speed_diff = 0;
            motor_l_duty(0);
            motor_r_duty(0);
            duct_enable = 0;
            buzzer_off();
        }
    }
    //Page Out
    if(key_num == 4 && menu_number == 0)
    {
        menu_number = 2;//进入run模式
        last_menu_cursor_pos = cursor_pos;
        cursor_pos = 0;
        last_cursor_pos = 0;
        ips114_clear();
    }
    else if(key_num == 4)
    {
        data_change_flag = 0; //退出数据修改状态
        menu_number = 0;
        cursor_pos = last_menu_cursor_pos;
        last_cursor_pos = last_menu_cursor_pos;
        ips114_clear();
    }
    //长按Data memory
    if(key_num == 5)
    {
        flash_write_mydata();
    }
}

void menu_0(void)
{
    ips114_show_string(8*2, 16*0, "Path:");
    ips114_show_int(8*7, 16*0, Path_Switch, 2);
    ips114_show_string(8*2, 16*1, "PID!!!");
    ips114_show_string(8*2, 16*2, "Run!!!");
}

void menu_1(void)
{
    //外环参数
    ips114_show_string(8*2,16*0,"out_P:");
    ips114_show_string(8*2,16*1,"out_D:");
    ips114_show_float(8*10,16*0,Outer_Ring_P,3,1);
    ips114_show_float(8*10,16*1,Outer_Ring_D,3,1);

    //内环参数
    ips114_show_string(8*2,16*2,"in_P:");
    ips114_show_string(8*2,16*3,"in_D:");
    ips114_show_float(8*10,16*2,Inner_Ring_P,1,3);
    ips114_show_float(8*10,16*3,Inner_Ring_D,1,3);

    //速度环参数
    ips114_show_string(8*2,16*4,"S_Kp:");
    ips114_show_string(8*2,16*5,"S_Ki:");
    ips114_show_float(8*10,16*4,kp_m,2,2);
    ips114_show_float(8*10,16*5,ki_m,2,2);

    //速度
    ips114_show_string(8*2,16*6,"T_speed:");
    ips114_show_int(8*10,16*6,target_speed,4);
    ips114_show_string(8*2,16*7,"duct_du:");
    ips114_show_int(8*10,16*7,duct_duty,5);

    //前瞻
    ips114_show_string(8*2,16*8,"M_range:");
    ips114_show_int(8*10,16*8,motor_range,5);
    ips114_show_string(8*2,16*9,"T_Point:");
    ips114_show_int(8*10,16*9,ImageStatus.TowPoint,2);
    ips114_show_string(8*2,16*10,"D_judge:");
    ips114_show_float(8*10,16*10,distance_judgment,2,2);

    if(data_change_flag)
    {
        switch(cursor_pos)
        {
            case 0: if(data_up_flag) {Outer_Ring_P += 10;data_up_flag = 0;} else if(data_down_flag) {Outer_Ring_P -= 10;data_down_flag = 0;}  break;
            case 1: if(data_up_flag) {Outer_Ring_D += 10;data_up_flag = 0;} else if(data_down_flag) {Outer_Ring_D -= 10;data_down_flag = 0;}  break;
            case 2: if(data_up_flag) {Inner_Ring_P += 0.001;data_up_flag = 0;} else if(data_down_flag) {Inner_Ring_P -= 0.001;data_down_flag = 0;}  break;
            case 3: if(data_up_flag) {Inner_Ring_D += 0.001;data_up_flag = 0;} else if(data_down_flag) {Inner_Ring_D -= 0.001;data_down_flag = 0;}  break;
            case 4: if(data_up_flag) {kp_m += 0.5;data_up_flag = 0;} else if(data_down_flag) {kp_m -= 0.5;data_down_flag = 0;}  break;
            case 5: if(data_up_flag) {ki_m += 0.01;data_up_flag = 0;} else if(data_down_flag) {ki_m -= 0.01;data_down_flag = 0;}  break;
            case 6: if(data_up_flag) {target_speed += 10;data_up_flag = 0;} else if(data_down_flag) {target_speed -= 10;data_down_flag = 0;}  break;
            case 7: if(data_up_flag) {duct_duty += 10;data_up_flag = 0;} else if(data_down_flag) {duct_duty -= 10;data_down_flag = 0;}  break;
            case 8: if(data_up_flag) {motor_range += 500;data_up_flag = 0;} else if(data_down_flag) {motor_range -= 500;data_down_flag = 0;}  break;
            case 9: if(data_up_flag) {ImageStatus.TowPoint += 1;data_up_flag = 0;} else if(data_down_flag) {ImageStatus.TowPoint -= 1;data_down_flag = 0;}  break;
            case 10: if(data_up_flag) {distance_judgment += 0.1;data_up_flag = 0;} else if(data_down_flag) {distance_judgment -= 0.1;data_down_flag = 0;}  break;
            default: break;
        }
    }
}

// 菜单显示：显示赛车运行状态（速度、角度、距离、路径、图像等）
void menu_2(void)
{
    ips114_show_string(8*2, 16*0, "Car_run:");
    ips114_show_int(8*10, 16*0,  stop_flag,1);

    ips114_show_string(8*2, 16*1, "Motor:");
    ips114_show_int(8*8, 16*1,  encoder.speedL,   3);                          // 左电机实际转速
    ips114_show_int(8*12, 16*1,  encoder.speedR,  3);                          // 右电机实际转速

    ips114_show_string(8*2,16*2,"Err:");
    ips114_show_int(8*6,16*2,  Err,     3);                                    // 当前偏差
    ips114_show_int(8*10,16*2,  target_speed,     3);                          // 目标速度

    if(Image_Solve_Finish_Flag)  // 防止初始时输出控制量，在图像解算完成的标志位之后，避免图像处理期间数据被意外修改
    {
        ips114_show_string(8*2, 16*3, "L:");
        ips114_show_int(8*4, 16*3,  ImageStatus.First_W_Point_L,     2);
        ips114_show_int(8*7, 16*3,  ImageStatus.First_H_Point_L,     2);
        ips114_show_int(8*10, 16*3,  ImageFrameDetect.spinodal_L_Ysite,     2);

        ips114_show_string(8*2, 16*4, "R:");
        ips114_show_int(8*4, 16*4,  ImageStatus.First_W_Point_R,     2);
        ips114_show_int(8*7, 16*4,  ImageStatus.First_H_Point_R,     2);
        ips114_show_int(8*10, 16*4,  ImageFrameDetect.spinodal_R_Ysite,     2);

        //直角检测标志位
        ips114_show_string(8*2, 16*5, "flag:");
        ips114_show_int(8*7, 16*5,  ImageFrameDetect.frame_top_flag,     1);       // 上边界标志
        ips114_show_int(8*9, 16*5,  ImageFrameDetect.frame_bottom_flag,     1);    // 下边界标志
        ips114_show_int(8*11, 16*5,  ImageFrameDetect.frame_left_flag,     1);     // 左边界标志
        ips114_show_int(8*13, 16*5,  ImageFrameDetect.frame_right_flag,     1);    // 右边界标志
    }
    
    ips114_show_string(8*2, 16*6, "fps:");
    ips114_show_int(8*6, 16*6,  image_solve_fps,     3);
    switch(ImageStatus.Road_type)
    {
        case 0: ips114_show_string(8*10, 16*6, "Normal"); break;
        case 1: ips114_show_string(8*10, 16*6, "AngleL"); break;
        case 2: ips114_show_string(8*10, 16*6, "AngleR"); break;
        case 3: ips114_show_string(8*10, 16*6, "Node_L"); break;
        case 4: ips114_show_string(8*10, 16*6, "Node_R"); break;
        case 5: ips114_show_string(8*10, 16*6, "Node_T"); break;
        case 6: ips114_show_string(8*10, 16*6, "Branch"); break;
        case 7: ips114_show_string(8*10, 16*6, "Cross "); break;
        case 8: ips114_show_string(8*10, 16*6, "Corect"); break;
        case 9: ips114_show_string(8*10, 16*6, "Protec"); break;
        default: break;
    }

    ips114_show_string(8*2, 16*7, "A:"); 
    ips114_show_float(8*4, 16*7, post.yaw_angle, 3,1);//当前角度
    ips114_show_float(8*10, 16*7, post.target_angle, 3,1);//目标角度

    // ips114_show_string(8*2, 16*8, "D:"); 
    ips114_show_int(8*2, 16*8, encoder.disA, 5);//当前距离
    ips114_show_int(8*9, 16*8, encoder.target_distance, 5);//目标距离

    ips114_show_string(8*2, 16*9, "path:"); 
    ips114_show_int(8*7, 16*9,  Path[Path_Switch].target_angle_index, 2);//当前路径索引

    ips114_show_int(8*11, 16*9,  Path[Path_Switch].memory_target_angle_turn[Path[Path_Switch].target_angle_index], 2);//目标转向
    ips114_show_gray_image(0, 16*10, mt9_binary_image[0], MT9_COMPRESS_W, MT9_COMPRESS_H, MT9_COMPRESS_W, MT9_COMPRESS_H, 1);
    image_line_show();
}

void menus_solve(void)//具体需要啥，可以根据我的逻辑去添加
{
    Page_In_Out(); //菜单进入与退出处理
    switch(menu_number)
    {
        case 0:
            menu_0();//主页面
        break;
        case 1://调参界面
            menu_1();//PID
        break;
        case 2://发车界面
            menu_2();//Running
        break;
        default:
            break;
    }
}

void Menu_Logic_Run(void)   // 菜单逻辑运行（周期性调用）
{
    battery_voltage_show();  // 底层：显示电池电压等信息

    cursor_move();  // 光标移动处理

    menus_solve();  // 顶层：菜单显示（速度、角度、距离、路径、图像等）

    key_clear_all_state();
}

