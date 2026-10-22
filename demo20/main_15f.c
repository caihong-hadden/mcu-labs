#include <STC15F2K60S2.H>
#include <intrins.h>

// ==================== 1. 引脚与硬件映射定义 ====================

// 前轮转向舵机 (保留原有舵机转向)
sbit SERVO_PIN = P3^0;

// 四驱电机 H 桥控制引脚 (实车精准校准)
sbit MOTOR_LR_IN1 = P1^0;  // 左后轮 (LR) 控制端 1
sbit MOTOR_LR_IN2 = P1^1;  // 左后轮 (LR) 控制端 2

sbit MOTOR_LF_IN1 = P1^2;  // 左前轮 (LF) 控制端 1
sbit MOTOR_LF_IN2 = P1^3;  // 左前轮 (LF) 控制端 2

sbit MOTOR_RF_IN1 = P1^4;  // 右前轮 (RF) 控制端 1
sbit MOTOR_RF_IN2 = P1^5;  // 右前轮 (RF) 控制端 2

sbit MOTOR_RR_IN1 = P1^6;  // 右后轮 (RR) 控制端 1
sbit MOTOR_RR_IN2 = P1^7;  // 右后轮 (RR) 控制端 2

// PS2 无线手柄接收器引脚 (用户接线: P0.0~P0.3)
sbit PS2_DAT = P0^0;  // 数据输入 DI (手柄->单片机, 利用 P0 板载排阻稳定上拉)
sbit PS2_CMD = P0^1;  // 命令输出 DO (单片机->手柄)
sbit PS2_CS  = P0^2;  // 片选信号 CS/ATT (低电平有效)
sbit PS2_CLK = P0^3;  // 时钟信号 SCK (空闲高电平，下降沿采样)

// ==================== 2. 底盘运动常数与掩码 ====================

#define MOTION_STOP    0
#define MOTION_FWD     1
#define MOTION_REV     2
#define MOTION_SPIN_L  3  // 原地坦克左转
#define MOTION_SPIN_R  4  // 原地坦克右转

// 端口状态掩码 (根据实车左右电机对装校准)
#define MASK_STOP        0x00  // 0000 0000B 四轮停转
#define MASK_FWD         0x5A  // 0101 1010B 四轮全前进: 右侧=01, 左侧=10
#define MASK_REV         0xA5  // 1010 0101B 四轮全后退: 右侧=10, 左侧=01
#define MASK_SPIN_LEFT   0x55  // 0101 0101B 原地左自旋: 右侧前进, 左侧后退
#define MASK_SPIN_RIGHT  0xAA  // 1010 1010B 原地右自旋: 右侧后退, 左侧前进

// 12MHz 晶振: 100us 定时初值
#define T0_TH0_INIT 0xFF
#define T0_TL0_INIT 0x9C

// 速度与角度基准
#define SPEED_CRUISE        85    // 巡航/前进冲刺速度 (0~100)
#define SPEED_SLOW          65    // 缓行速度 (0~100)
#define SPEED_BACK          85    // 倒车速度 (保持前后动力对称)
#define SERVO_TURN_DEG      20    // 转向偏转角度 (已调小至20度，避免轮胎碰电池)
#define TIME_BACK_STRAIGHT  3200  // 直线倒车时间 (ms): 补偿舞台秀前移量

// ==================== 3. 全局变量 ====================

unsigned char pwm_counter = 0;   // 0~199 (20ms 舵机周期)
unsigned char motor_duty = 0;    // 电机占空比 (0~100)
unsigned char car_dir = MOTION_STOP;
unsigned char servo_pulse = 15;  // 15 对应 1.5ms (90度居中)

// PS2 协议数据缓存
unsigned char PS2_Data[9];
unsigned int  Handkey = 0xFFFF;  // 16位按键状态 (按键按下对应位为0)
unsigned char PS2_Mode = 0;

#define PSB_LOSE            0    // 手柄未连接/掉线
#define PSB_REDLIGHT_MODE   1    // 红灯模式 (模拟双摇杆有效)
#define PSB_GREENLIGHT_MODE 2    // 绿灯模式 (离散按键有效)

// 按键掩码判定宏 (按下为 1, 松开为 0)
#define KEY_SELECT   ((~Handkey) & 0x0001)
#define KEY_L3       ((~Handkey) & 0x0002)
#define KEY_R3       ((~Handkey) & 0x0004)
#define KEY_START    ((~Handkey) & 0x0008)
#define KEY_UP       ((~Handkey) & 0x0010)
#define KEY_RIGHT    ((~Handkey) & 0x0020)
#define KEY_DOWN     ((~Handkey) & 0x0040)
#define KEY_LEFT     ((~Handkey) & 0x0080)
#define KEY_L2       ((~Handkey) & 0x0100)
#define KEY_R2       ((~Handkey) & 0x0200)
#define KEY_L1       ((~Handkey) & 0x0400)
#define KEY_R1       ((~Handkey) & 0x0800)
#define KEY_TRIANGLE ((~Handkey) & 0x1000)  // 绿 / 三角
#define KEY_CIRCLE   ((~Handkey) & 0x2000)  // 红 / 圆圈 (急刹车)
#define KEY_CROSS    ((~Handkey) & 0x4000)  // 蓝 / 叉号 (急刹车)
#define KEY_SQUARE   ((~Handkey) & 0x8000)  // 粉 / 方块

// 摇杆读取
#define PSS_RX       PS2_Data[5]  // 右摇杆 X (0~255)
#define PSS_RY       PS2_Data[6]  // 右摇杆 Y (0~255)
#define PSS_LX       PS2_Data[7]  // 左摇杆 X (0~255, 居中约128)
#define PSS_LY       PS2_Data[8]  // 左摇杆 Y (0~255, 居中约128)

// ==================== 4. 基础延时与定时器 ====================

void Delay_ms(unsigned int ms) {
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
}

void PS2_Delay_us(unsigned char us) {
    // STC15 为 1T 模式，单周期执行速度提升 12 倍，微秒延时增加 NOP
    unsigned char i;
    while (us--) {
        for (i = 0; i < 4; i++) {
            _nop_();
        }
    }
}

void Timer0_Init(void) {
    TMOD &= 0xF0;       
    TMOD |= 0x01;  // 模式1, 16位定时
    TH0 = T0_TH0_INIT;  
    TL0 = T0_TL0_INIT;
    ET0 = 1;            
    EA = 1;             
    TR0 = 1;            
}

// 舵机角度设置 (0 ~ 180 度，90度为居中回正)
void Set_Servo_Angle(unsigned char angle) {
    if (angle > 180) angle = 180;
    servo_pulse = 5 + (unsigned int)angle * 20 / 180;
}

// 电机状态底层控制
void Set_Car_Motion(unsigned char dir, unsigned char speed) {
    if (speed > 100) speed = 100;
    
    // 换向或停车时先切断输出，保护 H 桥管子
    if (dir != car_dir || speed == 0) {
        P1 = MASK_STOP;
    }

    motor_duty = speed;
    car_dir = (speed == 0) ? MOTION_STOP : dir;
}

// ==================== 5. 底盘运动控制 API ====================

void Car_Forward(unsigned char speed) {
    Set_Car_Motion(MOTION_FWD, speed);
}

void Car_Backward(unsigned char speed) {
    Set_Car_Motion(MOTION_REV, speed);
}

void Car_Turn_Left(unsigned char offset, unsigned char speed) {
    Set_Servo_Angle(90 - offset);
    Set_Car_Motion(MOTION_FWD, speed);
}

void Car_Turn_Right(unsigned char offset, unsigned char speed) {
    Set_Servo_Angle(90 + offset);
    Set_Car_Motion(MOTION_FWD, speed);
}

void Car_Turn_Left_Back(unsigned char offset, unsigned char speed) {
    Set_Servo_Angle(90 - offset);
    Set_Car_Motion(MOTION_REV, speed);
}

void Car_Turn_Right_Back(unsigned char offset, unsigned char speed) {
    Set_Servo_Angle(90 + offset);
    Set_Car_Motion(MOTION_REV, speed);
}

void Car_Spin_Left(unsigned char speed) {
    Set_Servo_Angle(90);
    Set_Car_Motion(MOTION_SPIN_L, speed);
}

void Car_Spin_Right(unsigned char speed) {
    Set_Servo_Angle(90);
    Set_Car_Motion(MOTION_SPIN_R, speed);
}

void Car_Stop(void) {
    Set_Car_Motion(MOTION_STOP, 0);
}

// 安全纯电急停：瞬间切断动力并回正舵机，杜绝电机暴力反冲导致的电源跌落与接收器复位
void Car_Emergency_Brake(void) {
    Car_Stop();
    Set_Servo_Angle(90);
}

// ==================== 6. PS2 无线手柄底层驱动 ====================

// 单字节 SPI 读写 (微调为 12us，提升长杜邦线抗干扰容限)
unsigned char PS2_Cmd(unsigned char CMD) {
    unsigned char i;
    unsigned char res = 0;
    for (i = 0; i < 8; i++) {
        if (CMD & 0x01) {
            PS2_CMD = 1;
        } else {
            PS2_CMD = 0;
        }
        CMD >>= 1;

        PS2_Delay_us(12);
        PS2_CLK = 0;
        PS2_Delay_us(12);

        PS2_DAT = 1;  // 读取前释放 P0.0 准备读取
        if (PS2_DAT) {
            res |= (1 << i);
        }

        PS2_CLK = 1;
        PS2_Delay_us(12);
    }
    return res;
}

// 轮询读取手柄完整 9 字节数据
void PS2_ReadData(void) {
    unsigned char i;
    PS2_CS = 0;
    PS2_Delay_us(25);

    PS2_Data[0] = PS2_Cmd(0x01);  // 握手头
    PS2_Data[1] = PS2_Cmd(0x42);  // 查询命令
    for (i = 2; i < 9; i++) {
        PS2_Data[i] = PS2_Cmd(0x00);
    }

    PS2_CS = 1;
    PS2_Delay_us(50); // CS 释放后留足 50us 间隔，让接收机内部 MCU 完成数据帧封包

    // 组合 16 位按键状态 (按键按下时对应 bit 为 0)
    Handkey = ((unsigned int)PS2_Data[4] << 8) | PS2_Data[3];

    // 检测工作模式 (0x73=红灯模拟模式, 0x41=绿灯数字模式)
    if (PS2_Data[1] == 0x73) {
        PS2_Mode = PSB_REDLIGHT_MODE;
    } else if (PS2_Data[1] == 0x41) {
        PS2_Mode = PSB_GREENLIGHT_MODE;
    } else {
        PS2_Mode = PSB_LOSE;
    }
}


void PS2_ShortPoll(void) {
    PS2_CS = 0;
    PS2_Delay_us(20);
    PS2_Cmd(0x01);
    PS2_Cmd(0x42);
    PS2_Cmd(0x00);
    PS2_Cmd(0x00);
    PS2_Cmd(0x00);
    PS2_CS = 1;
    PS2_Delay_us(50);
}

void PS2_EnterConfig(void) {
    PS2_CS = 0;
    PS2_Delay_us(20);
    PS2_Cmd(0x01);
    PS2_Cmd(0x43);
    PS2_Cmd(0x00);
    PS2_Cmd(0x01);
    PS2_Cmd(0x00);
    PS2_Cmd(0x00);
    PS2_Cmd(0x00);
    PS2_Cmd(0x00);
    PS2_Cmd(0x00);
    PS2_CS = 1;
    PS2_Delay_us(50);
}

void PS2_TurnOnAnalogMode(void) {
    PS2_CS = 0;
    PS2_Delay_us(20);
    PS2_Cmd(0x01);
    PS2_Cmd(0x44);
    PS2_Cmd(0x00);
    PS2_Cmd(0x01); // 模拟模式
    PS2_Cmd(0x03); // 允许用手柄 MODE 键切换红绿灯
    PS2_Cmd(0x00);
    PS2_Cmd(0x00);
    PS2_Cmd(0x00);
    PS2_Cmd(0x00);
    PS2_CS = 1;
    PS2_Delay_us(50);
}

void PS2_ExitConfig(void) {
    PS2_CS = 0;
    PS2_Delay_us(20);
    PS2_Cmd(0x01);
    PS2_Cmd(0x43);
    PS2_Cmd(0x00);
    PS2_Cmd(0x00);
    PS2_Cmd(0x5A);
    PS2_Cmd(0x5A);
    PS2_Cmd(0x5A);
    PS2_Cmd(0x5A);
    PS2_Cmd(0x5A);
    PS2_CS = 1;
    PS2_Delay_us(50);
}

void PS2_Init(void) {
    PS2_CLK = 1;
    PS2_CS  = 1;
    PS2_CMD = 1;
    PS2_DAT = 1;

    // 握手自适应初始化
    PS2_ShortPoll();
    Delay_ms(10);
    PS2_ShortPoll();
    Delay_ms(10);
    PS2_EnterConfig();
    Delay_ms(10);
    PS2_TurnOnAnalogMode();
    Delay_ms(10);
    PS2_ExitConfig();
    Delay_ms(10);
}

unsigned char lose_count = 0;

// 掉线自动重连监测机制：如果连续丢包，自动向接收机重发握手配置包
void PS2_Check_Reconnect(void) {
    lose_count++;
    if (lose_count > 20) {  // 约 0.5 秒持续未收到有效数据
        lose_count = 0;
        PS2_Init();         // 自动向接收机执行初始化握手与模式锁定
    }
}

// ==================== 7. 实时可打断的特技表演引擎 ====================

// 检查是否有任何遥控手动输入 (摇杆推开、急刹键、十字键等)
bit Check_Manual_Intervention(void) {
    if (PS2_Mode == PSB_REDLIGHT_MODE) {
        // 左摇杆推离中心死区 (油门前后或转向左右)
        if (PSS_LY < 90 || PSS_LY > 165 || PSS_LX < 90 || PSS_LX > 165) {
            return 1;
        }
        // 侧键自旋或圆圈/叉号急刹
        if (KEY_L1 || KEY_L2 || KEY_CIRCLE || KEY_CROSS) {
            return 1;
        }
    } else if (PS2_Mode == PSB_GREENLIGHT_MODE) {
        // 十字按键、侧键或急刹
        if (KEY_UP || KEY_DOWN || KEY_LEFT || KEY_RIGHT || KEY_L1 || KEY_L2 || KEY_CIRCLE || KEY_CROSS) {
            return 1;
        }
    }
    return 0;
}

// 毫秒级可打断延时: 表演中每 25ms 监测一次手柄 (40Hz黄金刷新率)，有操作立即返回 1 中断表演
bit Delay_Interruptible(unsigned int ms) {
    unsigned int elapsed = 0;
    while (elapsed < ms) {
        Delay_ms(25);
        elapsed += 25;

        PS2_ReadData();

        if (Check_Manual_Intervention()) {
            return 1;  // 收到摇杆/按键信号，立即打断
        }
    }
    return 0;  // 正常走完未被打断
}

// 原地萌系摆头打招呼 (支持实时打断)
bit Car_Wiggle_Dance(unsigned char times, unsigned int interval_ms) {
    unsigned char k;
    Car_Stop();
    for (k = 0; k < times; k++) {
        Set_Servo_Angle(90 - SERVO_TURN_DEG);
        if (Delay_Interruptible(interval_ms)) return 1;
        Set_Servo_Angle(90 + SERVO_TURN_DEG);
        if (Delay_Interruptible(interval_ms)) return 1;
    }
    Set_Servo_Angle(90);
    if (Delay_Interruptible(interval_ms)) return 1;
    return 0;
}

// S形神龙摆尾前进 (支持实时打断)
bit Car_Snake_Forward(unsigned char offset, unsigned int swing_ms, unsigned char swing_count, unsigned char speed) {
    unsigned char k;
    for (k = 0; k < swing_count; k++) {
        Car_Turn_Left(offset, speed);
        if (Delay_Interruptible(swing_ms)) return 1;
        Car_Turn_Right(offset, speed);
        if (Delay_Interruptible(swing_ms)) return 1;
    }
    Set_Servo_Angle(90);
    Set_Car_Motion(MOTION_FWD, speed);
    return 0;
}

// S形倒车蛇形后退 (支持实时打断)
bit Car_Snake_Backward(unsigned char offset, unsigned int swing_ms, unsigned char swing_count, unsigned char speed) {
    unsigned char k;
    for (k = 0; k < swing_count; k++) {
        Car_Turn_Left_Back(offset, speed);
        if (Delay_Interruptible(swing_ms)) return 1;
        Car_Turn_Right_Back(offset, speed);
        if (Delay_Interruptible(swing_ms)) return 1;
    }
    Set_Servo_Angle(90);
    Set_Car_Motion(MOTION_REV, speed);
    return 0;
}

// 脉冲律动冲刺 (支持实时打断)
bit Car_Pulse_Sprint(unsigned char high_speed, unsigned char low_speed, unsigned char cycles) {
    unsigned char k;
    Set_Servo_Angle(90);
    for (k = 0; k < cycles; k++) {
        Set_Car_Motion(MOTION_FWD, high_speed);
        if (Delay_Interruptible(600)) return 1;
        Set_Car_Motion(MOTION_FWD, low_speed);
        if (Delay_Interruptible(400)) return 1;
    }
    return 0;
}

// 一键自动表演特技 (表演过程中随时可被摇杆抢占打断)
void Performance_Mode_Symmetric(void) {
    // 阶段1: 开场打招呼
    if (Car_Wiggle_Dance(2, 280)) goto abort_perf;
    if (Delay_Interruptible(400)) goto abort_perf;

    // 阶段2: 蛇形 S 弯前进
    if (Car_Snake_Forward(SERVO_TURN_DEG, 500, 2, SPEED_CRUISE)) goto abort_perf;

    // 阶段3: 高速直冲
    Car_Forward(SPEED_CRUISE);
    if (Delay_Interruptible(1200)) goto abort_perf;

    // 阶段4: 脉冲律动冲刺
    if (Car_Pulse_Sprint(SPEED_CRUISE, SPEED_SLOW, 2)) goto abort_perf;

    // 阶段5: 中场驻车打招呼
    Car_Stop();
    Set_Servo_Angle(90);
    if (Delay_Interruptible(400)) goto abort_perf;
    if (Car_Wiggle_Dance(1, 300)) goto abort_perf;
    if (Delay_Interruptible(500)) goto abort_perf;

    // 阶段6: 倒车返航
    Car_Backward(SPEED_BACK);
    if (Delay_Interruptible(TIME_BACK_STRAIGHT)) goto abort_perf;

    // 阶段7: 倒车 S 弯返航
    if (Car_Snake_Backward(SERVO_TURN_DEG, 500, 2, SPEED_BACK)) goto abort_perf;

    // 阶段8: 终点谢幕
    Car_Stop();
    Set_Servo_Angle(90);
    if (Delay_Interruptible(400)) goto abort_perf;
    if (Car_Wiggle_Dance(3, 200)) goto abort_perf;
    Car_Stop();
    return;

abort_perf:
    // 收到摇杆指令，立即退出表演，直接转入遥控模式
    return;
}

// ==================== 8. 主逻辑控制循环 ====================

void main(void) {
    unsigned char lx, ly;

    P1 = 0x00;
    SERVO_PIN = 0;
    
    Timer0_Init();
    
    // 上电自检：舵机居中 90 度，全车静止
    Set_Servo_Angle(90);
    Car_Stop();
    Delay_ms(1000);

    // 初始化 PS2 接收器
    PS2_Init();

    while (1) {
        PS2_ReadData();

        // 1. 手柄未连接/掉线安全保护与自动重连握手
        if (PS2_Mode == PSB_LOSE) {
            Car_Stop();
            Set_Servo_Angle(90);
            PS2_Check_Reconnect();  // 自动重新向接收机发送初始化配置帧
            Delay_ms(25);
            continue;
        } else {
            lose_count = 0;         // 只要手柄通信正常，立即清空重连计数器
        }

        // 2. 【最高优先级】紧急制动 (按住圆圈 ○ 或叉号 ✕ 键)
        // 瞬间全停驻车并回正舵机，只要按住就死死刹停，强力压制任何摇杆推力！
        if (KEY_CIRCLE || KEY_CROSS) {
            Car_Emergency_Brake();
            while ((KEY_CIRCLE || KEY_CROSS) && (PS2_Mode != PSB_LOSE)) {
                Car_Stop();
                Set_Servo_Angle(90);
                Delay_ms(25);
                PS2_ReadData();
            }
            continue;
        }

        // 3. 一键触发全套回原点表演秀 (按手柄 START 键)
        // 表演过程中推动摇杆或按急停键可瞬间打断退出
        if (KEY_START) {
            Performance_Mode_Symmetric();
            continue;
        }

        // 4. 特技快捷键 (粉/方块键: 神龙摆尾; 绿/三角键: 原地摆头打招呼)
        if (KEY_SQUARE) {
            Car_Snake_Forward(SERVO_TURN_DEG, 400, 2, SPEED_CRUISE);
            continue;
        } else if (KEY_TRIANGLE) {
            Car_Wiggle_Dance(2, 250);
            continue;
        }

        // 5. 红灯模式 (模拟双摇杆遥控)
        if (PS2_Mode == PSB_REDLIGHT_MODE) {
            lx = PSS_LX;  // 左摇杆左右转向 (0~255)
            ly = PSS_LY;  // 左摇杆前后油门 (0~255)

            // --- 舵机转向控制 (左摇杆 X 轴比例平滑转向) ---
            if (lx < 100) {
                // 向左推摇杆 -> 舵机向左比例偏转
                Set_Servo_Angle(90 - (unsigned int)(128 - lx) * SERVO_TURN_DEG / 128);
            } else if (lx > 156) {
                // 向右推摇杆 -> 舵机向右比例偏转
                Set_Servo_Angle(90 + (unsigned int)(lx - 128) * SERVO_TURN_DEG / 128);
            } else {
                // 摇杆居中死区 -> 舵机居中 90 度
                Set_Servo_Angle(90);
            }

            // --- 动力驱动控制 (左摇杆 Y 轴 & L1/L2 自旋键) ---
            if (KEY_L1) {
                Car_Spin_Left(SPEED_CRUISE);   // L1 原地坦克左转
            } else if (KEY_L2) {
                Car_Spin_Right(SPEED_CRUISE);  // L2 原地坦克右转
            } else if (ly < 90) {
                Car_Forward(SPEED_CRUISE);     // 推杆向前 -> 前进
            } else if (ly > 165) {
                Car_Backward(SPEED_BACK);      // 拉杆向后 -> 倒车
            } else {
                Car_Stop();                    // 油门归中 -> 停车
            }
        }
        // 6. 绿灯模式 (十字方向键离散遥控)
        else if (PS2_Mode == PSB_GREENLIGHT_MODE) {
            if (KEY_UP) {
                if (KEY_LEFT) {
                    Set_Servo_Angle(90 - SERVO_TURN_DEG);  // 前进左转
                } else if (KEY_RIGHT) {
                    Set_Servo_Angle(90 + SERVO_TURN_DEG);  // 前进右转
                } else {
                    Set_Servo_Angle(90);                   // 直行前进
                }
                Car_Forward(SPEED_CRUISE);
            } else if (KEY_DOWN) {
                if (KEY_LEFT) {
                    Set_Servo_Angle(90 - SERVO_TURN_DEG);  // 倒车左转
                } else if (KEY_RIGHT) {
                    Set_Servo_Angle(90 + SERVO_TURN_DEG);  // 倒车右转
                } else {
                    Set_Servo_Angle(90);                   // 直行后退
                }
                Car_Backward(SPEED_BACK);
            } else if (KEY_LEFT) {
                Set_Servo_Angle(90);
                Car_Spin_Left(SPEED_CRUISE);               // 原地左自旋
            } else if (KEY_RIGHT) {
                Set_Servo_Angle(90);
                Car_Spin_Right(SPEED_CRUISE);              // 原地右自旋
            } else if (KEY_L1) {
                Set_Servo_Angle(90);
                Car_Spin_Left(SPEED_CRUISE);
            } else if (KEY_L2) {
                Set_Servo_Angle(90);
                Car_Spin_Right(SPEED_CRUISE);
            } else {
                Car_Stop();
                Set_Servo_Angle(90);                       // 无操作自动停车回正
            }
        }

        Delay_ms(20);  // 50Hz 轮询手柄
    }
}

// ==================== 9. 定时器 0 中断服务 ====================

void Timer0_ISR(void) interrupt 1 {
    // STC15 16位自动重装载模式无需在中断内手动重载 TH0/TL0，硬件自动装载保证零抖动

    pwm_counter++;
    if (pwm_counter >= 200) {  
        pwm_counter = 0;
    }

    // 前轮转向舵机通道 (50Hz 周期, 20ms)
    if (pwm_counter < servo_pulse) {
        SERVO_PIN = 1;
    } else {
        SERVO_PIN = 0;
    }

    // 四驱电机调速通道 (100Hz 周期, 10ms)
    if ((pwm_counter % 100) < motor_duty) {
        switch (car_dir) {
            case MOTION_FWD:    P1 = MASK_FWD; break;
            case MOTION_REV:    P1 = MASK_REV; break;
            case MOTION_SPIN_L: P1 = MASK_SPIN_LEFT; break;
            case MOTION_SPIN_R: P1 = MASK_SPIN_RIGHT; break;
            default:            P1 = MASK_STOP; break;
        }
    } else {
        P1 = MASK_STOP;
    }
}