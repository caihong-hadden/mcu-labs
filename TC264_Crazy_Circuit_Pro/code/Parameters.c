/*
 * Parameters.c
 *
 *  全锟街诧拷锟斤拷锟斤拷始锟斤拷模锟斤拷 锟斤拷 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷路锟斤拷锟斤拷预锟斤拷锟斤拷锟捷ｏ拷锟角讹拷锟斤拷锟叫★拷锟斤拷锟斤拷锟斤拷锟叫★拷转锟斤拷锟斤拷锟斤拷锟叫ｏ拷锟斤拷
 *  锟皆硷拷系统锟杰筹拷始锟斤拷锟斤拷锟斤拷锟斤拷默锟较诧拷锟斤拷锟斤拷锟斤拷
 *
 *  Created on: 2026锟斤拷1锟斤拷18锟斤拷
 *      Author: Administrator
 */

#include "zf_common_headfile.h"

int Path_Switch = 0;// 路锟斤拷选锟斤拷0=锟斤拷目一锟斤拷1=锟斤拷目锟斤拷锟斤拷2=锟斤拷目锟斤拷

// ============================================================
// 锟斤拷目一锟斤拷目锟斤拷嵌锟斤拷锟斤拷校锟�12锟斤拷元锟截ｏ拷
// 小锟斤拷锟斤拷锟斤拷每锟斤拷目锟斤拷锟绞憋拷锟斤拷锟斤拷锟斤拷锟侥撅拷锟皆角度ｏ拷锟斤拷位锟斤拷锟饺ｏ拷
// ============================================================
int Subject_One_Path_tratget_angle[26] ={
        90,180,90,360,90,
        90,360,270,180,90,
        180,90,180,270,270,
        180,90,90,90,360,
        270,270,180,270,270,
        360};

// ============================================================
// 锟斤拷目锟斤拷锟斤拷目锟斤拷嵌锟斤拷锟斤拷校锟�14锟斤拷元锟截ｏ拷
// ============================================================
int Subject_Two_Path_tratget_angle[26] ={270,270,270,180,270,
                                         360,90,180,270,180,
                                         180,90,180,270,180,
                                         90,360,270,270,360,
                                         360,90,90,180,90,
                                         360};

// ============================================================
// 锟斤拷目锟斤拷锟斤拷目锟斤拷嵌锟斤拷锟斤拷校锟�19锟斤拷元锟截ｏ拷
// ============================================================
int Subject_Three_Path_tratget_angle[19] = {90,180,180,90,360,
                                            360,90,180,90,360,
                                            270,270,360,270,180,
                                            270,180,270,360};

// ============================================================
// 锟斤拷目一锟斤拷目锟斤拷锟斤拷锟斤拷锟斤拷校锟�13锟斤拷元锟截ｏ拷锟饺角讹拷锟斤拷锟叫讹拷1锟斤拷锟斤拷锟斤拷锟斤拷锟揭伙拷锟轿ｏ拷锟斤拷锟斤拷耄�
// 锟斤拷锟斤拷目锟斤拷锟街拷锟斤拷锟斤拷驶锟斤拷锟诫，使锟斤拷CM_TO_ENCODER锟疥将锟斤拷锟斤拷转锟斤拷为锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
// ============================================================
int Subject_One_Path_memory_target_distance[27] = {
        CM_TO_ENCODER(160),CM_TO_ENCODER(55),CM_TO_ENCODER(135),CM_TO_ENCODER(45),CM_TO_ENCODER(90),
        CM_TO_ENCODER(75),CM_TO_ENCODER(60),CM_TO_ENCODER(45),CM_TO_ENCODER(120),CM_TO_ENCODER(40),
        CM_TO_ENCODER(60),CM_TO_ENCODER(45),CM_TO_ENCODER(60),CM_TO_ENCODER(45),CM_TO_ENCODER(120),
        CM_TO_ENCODER(45),CM_TO_ENCODER(85),CM_TO_ENCODER(45),CM_TO_ENCODER(70),CM_TO_ENCODER(60),
        CM_TO_ENCODER(45),CM_TO_ENCODER(60),CM_TO_ENCODER(60),CM_TO_ENCODER(45),CM_TO_ENCODER(45),
        CM_TO_ENCODER(55),CM_TO_ENCODER(180)};

// ============================================================
// 锟斤拷目锟斤拷锟斤拷目锟斤拷锟斤拷锟斤拷锟斤拷校锟�15锟斤拷元锟截ｏ拷
// ============================================================
int Subject_Two_Path_memory_target_distance[27] = {
        CM_TO_ENCODER(160),CM_TO_ENCODER(65),CM_TO_ENCODER(55),CM_TO_ENCODER(60),CM_TO_ENCODER(45),
        CM_TO_ENCODER(60),CM_TO_ENCODER(45),CM_TO_ENCODER(60),CM_TO_ENCODER(45),CM_TO_ENCODER(60),
        CM_TO_ENCODER(30),CM_TO_ENCODER(45),CM_TO_ENCODER(60),CM_TO_ENCODER(45),CM_TO_ENCODER(60),
        CM_TO_ENCODER(45),CM_TO_ENCODER(120),CM_TO_ENCODER(45),CM_TO_ENCODER(60),CM_TO_ENCODER(60),
        CM_TO_ENCODER(45),CM_TO_ENCODER(45),CM_TO_ENCODER(120),CM_TO_ENCODER(45),CM_TO_ENCODER(135),
        CM_TO_ENCODER(55),CM_TO_ENCODER(170)};
// ============================================================
// 锟斤拷目锟斤拷锟斤拷目锟斤拷锟斤拷锟斤拷锟斤拷校锟�20锟斤拷元锟截ｏ拷
// ============================================================
int Subject_Three_Path_memory_target_distance[6] = {  CM_TO_ENCODER(160),CM_TO_ENCODER(55),CM_TO_ENCODER(135),CM_TO_ENCODER(45),CM_TO_ENCODER(90),CM_TO_ENCODER(20)};

// ============================================================
// 锟斤拷目一锟斤拷转锟斤拷锟斤拷锟斤拷锟叫ｏ拷12锟斤拷元锟截ｏ拷
// 'R' = 锟斤拷转锟斤拷'L' = 锟斤拷转锟斤拷'N' = 锟斤拷转锟斤拷直锟叫ｏ拷
// ============================================================
uint8_t Subject_One_Path_memory_target_angle_turn[26] = {
        'R','R','L','L','R',
        'N','L','L','L','L',
        'R','L','R','R','N',
        'L','L','N','N','L',
        'L','N','L','R','N',
        'R'};// R=锟斤拷转锟斤拷L=锟斤拷转锟斤拷N=锟斤拷转

// ============================================================
// 锟斤拷目锟斤拷锟斤拷转锟斤拷锟斤拷锟斤拷锟叫ｏ拷14锟斤拷元锟截ｏ拷
// ============================================================
uint8_t Subject_Two_Path_memory_target_angle_turn[26] ={
        'L','N','N','L','R',
        'R','R','R','R','L',
        'N','L','R','R','L',
        'L','L','L','N','R',
        'N','R','N','R','L',
        'L'};// R=锟斤拷转锟斤拷L=锟斤拷转锟斤拷N=锟斤拷转

// ============================================================
// 锟斤拷目锟斤拷锟斤拷转锟斤拷锟斤拷锟斤拷锟叫ｏ拷19锟斤拷元锟截ｏ拷
// ============================================================
uint8_t Subject_Three_Path_memory_target_angle_turn[5] = {'R','R','L','L','R'};

// 系统锟杰筹拷始锟斤拷锟斤拷锟斤拷
// 锟斤拷锟斤拷锟斤拷顺锟斤拷锟斤拷锟轿筹拷始锟斤拷锟斤拷锟斤拷锟斤拷锟借：锟斤拷锟斤拷锟斤拷锟斤拷锟竭★拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷幕锟斤拷锟斤拷锟斤拷头锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷恰锟紸DC锟斤拷Gamma锟斤拷LED
// 然锟斤拷锟斤拷锟絇ath_Switch锟斤拷预锟斤拷路锟斤拷锟斤拷锟捷硷拷锟截碉拷Path锟结构锟藉，锟斤拷锟斤拷锟斤拷PIT锟斤拷时锟叫讹拷
void All_Init(void)
{
    Parameters_Init();// 锟斤拷锟斤拷锟斤拷始锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟角帮拷妫凤拷锟斤拷锟斤拷锟绞癸拷玫谋锟斤拷锟斤拷锟斤拷殉锟绞硷拷锟�
    Wireless_init_flag = wifi_spi_init("SunnyRainbow", "1234567891");   // 锟斤拷始锟斤拷锟斤拷锟竭达拷锟斤拷
    buzzer_init();                               // 锟斤拷始锟斤拷锟斤拷锟斤拷锟斤拷
    ips114_init();                               // 锟斤拷始锟斤拷IPS114锟斤拷幕
    ips114_show_string(0, 0, "mt9v03x init.");
    while(1)
    {
        if(mt9v03x_double_init(mt9v03x_1))      // 锟斤拷始锟斤拷双目锟斤拷锟斤拷头锟斤拷锟斤拷失锟斤拷锟斤拷锟斤拷锟斤拷
            ips114_show_string(0, 16, "mt9v03x reinit.");
        else
            break;
        system_delay_ms(500);                    // 锟斤拷时0.5锟斤拷锟斤拷锟斤拷锟�
    }
    ips114_show_string(0, 16, "init success.");
    ips114_clear();
    motor_init();                                // 锟斤拷始锟斤拷锟斤拷锟�
    encoder_init();                              // 锟斤拷始锟斤拷锟斤拷锟斤拷锟斤拷
    gyro_init();                                 // 锟斤拷始锟斤拷 IMU660RA 锟斤拷锟斤拷锟斤拷
    adc_init(ADC0_CH11_A11, ADC_12BIT);          // 锟斤拷始锟斤拷 ADC
    gamma_init(gamma_value);                     // 锟斤拷始锟斤拷 Gamma 校锟斤拷
    LED_init();                                  // 锟斤拷始锟斤拷LED锟斤拷
    key_init(10);                               // 锟斤拷始锟斤拷锟斤拷锟斤拷

    // 锟斤拷锟斤拷锟斤拷锟斤拷路锟斤拷锟侥筹拷锟饺ｏ拷目锟斤拷锟斤拷锟斤拷锟斤拷锟�
    Path[0].path_length = 26;
    Path[1].path_length = 26;
    Path[2].path_length = 5;

    // 锟斤拷锟斤拷目一/锟斤拷/锟斤拷锟斤拷预锟斤拷嵌锟斤拷锟斤拷屑锟斤拷氐锟絇ath锟结构锟斤拷
    for (int i = 0; i < Path[0].path_length; i++)      Path[0].memory_target_angle[i] = Subject_One_Path_tratget_angle[i];
    for (int i = 0; i < Path[1].path_length; i++)      Path[1].memory_target_angle[i] = Subject_Two_Path_tratget_angle[i];
    for (int i = 0; i < Path[2].path_length; i++)    Path[2].memory_target_angle[i] = Subject_Three_Path_tratget_angle[i];

    // 锟斤拷锟斤拷目一/锟斤拷/锟斤拷锟斤拷预锟斤拷锟斤拷锟斤拷锟斤拷屑锟斤拷氐锟絇ath锟结构锟藉（锟斤拷锟斤拷锟斤拷锟叫比角讹拷锟斤拷锟叫讹拷1锟斤拷元锟截ｏ拷
    for (int i = 0; i < Path[0].path_length+1; i++)      Path[0].memory_target_distance[i] = Subject_One_Path_memory_target_distance[i];
    for (int i = 0; i < Path[1].path_length+1; i++)      Path[1].memory_target_distance[i] = Subject_Two_Path_memory_target_distance[i];
    for (int i = 0; i < Path[2].path_length+1; i++)    Path[2].memory_target_distance[i] = Subject_Three_Path_memory_target_distance[i];

    // 锟斤拷锟斤拷目一/锟斤拷/锟斤拷锟斤拷预锟斤拷转锟斤拷锟斤拷锟斤拷锟叫硷拷锟截碉拷Path锟结构锟斤拷
    for (int i = 0; i < Path[0].path_length; i++)      Path[0].memory_target_angle_turn[i] = Subject_One_Path_memory_target_angle_turn[i];
    for (int i = 0; i < Path[1].path_length; i++)      Path[1].memory_target_angle_turn[i] = Subject_Two_Path_memory_target_angle_turn[i];
    for (int i = 0; i < Path[2].path_length; i++)    Path[2].memory_target_angle_turn[i] = Subject_Three_Path_memory_target_angle_turn[i];

    // 默锟较诧拷使锟斤拷EEPROM锟斤拷锟捷ｏ拷直锟斤拷使锟斤拷默锟较诧拷锟斤拷锟斤拷锟斤拷锟斤拷使锟斤拷EEPROM锟斤拷取锟斤拷锟斤拷锟斤拷注锟酵硷拷锟缴ｏ拷
    //   锟阶达拷使锟斤拷时EEPROM为锟秸ｏ拷锟斤拷写锟斤拷默锟较诧拷锟斤拷锟斤拷之锟斤拷每锟轿匡拷锟斤拷锟斤拷EEPROM锟斤拷取锟斤拷锟斤拷
    // data_init();
    // // flash_erase_page(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);   // 锟斤拷锟斤拷EEPROM页
    LED_on(Path_Switch);   // 锟斤拷LED指示锟斤拷前选锟斤拷锟铰凤拷锟�

    // 锟叫断筹拷始锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷谐锟绞硷拷锟街拷螅锟斤拷锟斤拷卸锟斤拷蟹锟斤拷锟轿达拷锟绞硷拷锟斤拷谋锟斤拷锟�
    pit_ms_init(CCU60_CH0, 5);               // 锟斤拷始锟斤拷 PIT 锟斤拷时锟叫断ｏ拷锟斤拷锟斤拷 5ms
}

// 锟斤拷锟斤拷锟斤拷始锟斤拷锟斤拷锟斤拷
// 锟斤拷锟斤拷锟斤拷锟斤拷全锟斤拷锟斤拷锟叫诧拷锟斤拷锟斤拷默锟斤拷值锟斤拷锟斤拷锟斤拷锟接撅拷锟斤拷锟斤拷锟斤拷PID锟斤拷锟斤拷锟斤拷锟劫度诧拷锟斤拷锟斤拷元锟截硷拷饪拷氐锟�
void Parameters_Init(void)
{
    // 路锟斤拷选锟斤拷默锟斤拷锟斤拷目一
    Path_Switch = 0;

    // ======== 锟接撅拷锟斤拷锟斤拷锟斤拷锟� ========
    Gain_High = 9;                       // 图锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
    Gain_Low = -3;                       // 图锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
    whitehigh = 0;                       // 锟阶碉拷锟斤拷值
    ImageStatus.Road_type = Normol;      // 默锟较碉拷路锟斤拷锟斤拷
    ImageScanInterval_small = 3;         // 扫锟借范围锟斤拷锟竭斤拷锟斤拷锟斤拷锟絀mageScanInterval
    ImageStatus.TowPoint = 17;           // 前瞻锟斤拷权锟斤拷
    OSTU_Low = 5;                        // 锟斤拷锟斤拷锟街碉拷锟斤拷锟�
    OSTU_High = 120;                     // 锟斤拷锟斤拷锟街碉拷锟斤拷锟�
    gamma_value = 0.5;                   // Gamma校锟斤拷系锟斤拷
    distance_judgment = 0;               // 锟斤拷锟斤拷锟叫讹拷锟斤拷值锟斤拷锟斤拷位cm锟斤拷锟斤拷0锟斤拷示锟斤拷使锟矫撅拷锟斤拷锟叫讹拷

    // ======== 锟斤拷锟狡诧拷锟斤拷 ========
    Outer_Ring_P = 300;                  // 锟解环锟斤拷锟角度伙拷锟斤拷锟斤拷锟斤拷系锟斤拷350
    Outer_Ring_D = 85;                  // 锟解环锟斤拷锟角度伙拷锟斤拷微锟斤拷系锟斤拷100
    Inner_Ring_P = 0.069;                // 锟节伙拷锟斤拷锟斤拷锟劫度伙拷锟斤拷锟斤拷锟斤拷系锟斤拷0.05
    Inner_Ring_D = 0.046;                // 锟节伙拷锟斤拷锟斤拷锟劫度伙拷锟斤拷微锟斤拷系锟斤拷0.036
    kp_m = 6.6;                          // 锟斤拷锟絇ID锟斤拷锟斤拷系锟斤拷6.6
    ki_m = 0.26;                          // 锟斤拷锟絇ID锟斤拷锟斤拷系锟斤拷0.26
    kd_m = 0.1;                          // speed loop D
    target_speed = 330;                  // target speed
    turn_speed = 300;                     // turn/element target speed
    outer_d_error_limit = 8;            // D sees at most 8 pixels per 10ms update
    outer_output_limit = 12000;         // full-scale P authority, abnormal spike guard
    speed_diff_limit_percent = 100;     // max diff equals base speed; no reverse target
    high_speed_forward_only = 1;        // race mode: forward drive only while running
    element_entry_step = 6;             // reach full element steering in about 35ms
    element_release_step = 4;           // fast but continuous release in about 35-50ms
    element_release_angle = 50;         // soften only during the final approach to target yaw
    element_release_min_percent = 70;   // keep strong steering until the element exit condition
    element_exit_guard_ticks = 8;       // cover 40ms after Road_type returns to normal
    element_high_yaw_release_threshold = 2500; // damp only high residual rotation after exit
    element_high_yaw_release_step = 12; // return steering in about 15-20ms when yaw remains high
    element_high_yaw_release_angle = 40; // start neutral release before the 25-degree exit threshold
    t_node_prebrake_path = 0;           // path 0 contains the measured high-speed left T node
    t_node_prebrake_index = 16;         // target only node 16; global race speed is unchanged
    t_node_prebrake_start = CM_TO_ENCODER(35);
    t_node_prebrake_end = CM_TO_ENCODER(55);
    t_node_speed = 260;                 // reduce 0/600 wheel target to about 0/520 in the T node
    t_node_exit_hold_ticks = 12;        // hold 260 for 60ms after advancing to node 17
    route_map_enable = 1;               // use the fixed course topology for path 0
    route_approach_speed = 300;         // node 11/12 recognition approach speed only
    duct_duty = 3500;                 // duct target duty
    duct_actual_duty = 0;             // actual duct output starts from 0
    duct_enable = 0;                  // duct switch starts off, independent from wheels
    duct_ramp_step = 10;              // about 2s from 0 to 4000
    motor_range = 8888;                  // 锟斤拷锟絇WM锟斤拷围

    // ======== 元锟截硷拷饪拷兀锟�1=锟斤拷锟矫ｏ拷0=锟斤拷锟矫ｏ拷 ========
    Element_lock.RightAngle_L = 1;       // 锟斤拷直锟角硷拷锟�
    Element_lock.RightAngle_R = 1;       // 锟斤拷直锟角硷拷锟�
    Element_lock.Node_L = 1;             // 锟斤拷锟斤拷锟斤拷
    Element_lock.Node_R = 1;             // 锟揭斤拷锟斤拷锟�
    Element_lock.Node_T = 1;             // T锟酵斤拷锟斤拷锟�
    Element_lock.Branch = 1;             // 锟斤拷支路锟斤拷锟�
    Element_lock.Cross  = 1;             // 十锟斤拷路锟节硷拷锟�
    Element_lock.Protect = 1;            // 锟斤拷锟斤拷锟斤拷志
}


