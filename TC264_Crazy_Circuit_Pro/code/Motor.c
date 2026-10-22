/*
 * Motor.c
 *
 *  Created on: 2026��1��18��
 *      Author: Administrator
 */

#include "zf_common_headfile.h"

ERROR error = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
MOTOR motor = {0,0,0,0,0,0,0,0,0,0};

float kp_m = 0;
float ki_m = 0;
float kd_m = 0;
int ki_out = 0;
int kp_out = 0;
int kd_out = 0;
int pid_out = 0;
int motor_range = 0;        // PWM����޷����ٶȻ������Χ���ƣ�
int target_speed = 0;       // Ŀ���ٶ�
int slow_start_speed = 0;   // start speed
int turn_speed = 80;         // turn/element target speed
int outer_d_error_limit = 0;        // outer-loop D input limiter, pixels per update
int outer_output_limit = 0;         // absolute target yaw-rate limit
int speed_diff_limit_percent = 100; // differential limit relative to base speed
int high_speed_forward_only = 1;    // race mode: do not reverse a wheel while running
int element_entry_step = 0;         // virtual-center attack step per 5ms control tick
int element_release_step = 0;       // virtual-center release step per 5ms control tick
int element_release_angle = 0;      // begin reducing forced steering below this angle
int element_release_min_percent = 100; // steering authority kept at element exit threshold
int element_exit_guard_ticks = 0;   // keep exit shaping alive across the final forced image frame
int element_high_yaw_release_threshold = 0; // residual yaw-rate threshold after element exit
int element_high_yaw_release_step = 0;      // fast release step while residual yaw is high
int element_high_yaw_release_angle = 0;     // allow fast release this close to target yaw
int steering_center = ImageSensorMid; // center actually used by the outer steering loop
int t_node_prebrake_path = 0;        // route with the high-speed T-node approach
int t_node_prebrake_index = 16;      // node index requiring route-specific braking
int t_node_prebrake_start = 0;       // encoder distance where linear braking starts
int t_node_prebrake_end = 0;         // encoder distance where T-node speed is reached
int t_node_speed = 0;                // base speed through the selected T node
int t_node_exit_hold_ticks = 0;      // keep T-node speed briefly after node advance
volatile int race_command_speed = 0;
volatile int race_t_node_exit_hold_remaining = 0;
volatile int race_wheel_speed_avg = 0;
volatile int race_wheel_overshoot_percent = 0;
volatile int race_yaw_tracking_percent = 0;
volatile int race_slip_score = 0;
volatile int race_high_yaw_release_active = 0; // 0=off, 1=before exit, 2=after exit
volatile int race_high_yaw_release_count = 0;
int duct_duty = 0;          // duct target duty, controlled by VOFA
int duct_actual_duty = 0; // duct actual ramp output
int duct_enable = 0;      // duct switch, independent from wheel start
int duct_ramp_step = 10;  // about 2s from 0 to 4000 at 5ms tick
static int duct_last_stop_flag = 1; // close duct on every running-to-stop transition
volatile RaceState_e race_state = RACE_STATE_STOPPED;
volatile int race_state_ticks = 0;
volatile uint32_t race_tick_5ms = 0;
volatile RaceStopReason_e race_stop_reason = RACE_STOP_NONE;
volatile int race_stop_node_index = 0;
volatile int race_stop_distance = 0;
volatile int race_stop_target_distance = 0;
volatile int race_stop_road_type = Normol;
volatile int race_protect_black_frames = 0;
volatile int race_protect_grace_ticks = 0;
volatile int race_stop_protect_black_frames = 0;
volatile int race_stop_protect_offline = 0;
volatile int race_stop_protect_pixel_left = 0;
volatile int race_stop_protect_pixel_mid = 0;
volatile int race_stop_protect_pixel_right = 0;

static unsigned char Steering_Target_Ready = 0;
static unsigned char Steering_Element_Mode = 0; // 0=normal, 1=turning element, 2=exit blend
static int Steering_Turn_Direction = 0;          // -1=left, 1=right
static int Steering_Exit_Guard = 0;
static unsigned char Steering_High_Yaw_Release_Was_Active = 0;
static unsigned char T_Node_Speed_Active = 0;

// �����ʼ��������PWMͨ���ͷ����������
void motor_init(void)
{
    pwm_init(MOTOR_L_PWM, 17000, 0);        // ����PWM��ʼ����Ƶ��17KHz
    pwm_init(MOTOR_R_PWM, 17000, 0);        // �ҵ��PWM��ʼ����Ƶ��17KHz
    pwm_init(MOTOR_DUCT_PIN, 17000, 0);     // ��ˢ���PWM��ʼ����ɢ�ȷ��ȣ�

    gpio_init(MOTOR_L_DIR, GPO, GPIO_HIGH, GPO_PUSH_PULL);  // ������������
    gpio_init(MOTOR_R_DIR, GPO, GPIO_HIGH, GPO_PUSH_PULL);  // �ҵ����������
}

// ��������ռ�ձȣ���ֵΪǰ������ֵΪ����
void motor_l_duty(int duty)
{
    if(duty >= 0)
    {
        gpio_set_level(MOTOR_L_DIR,GPIO_LOW);   // ��ת����
    }
    else
    {
        gpio_set_level(MOTOR_L_DIR,GPIO_HIGH);    // ��ת����
        duty = -duty;                               // ȡ����ֵ
    }
    pwm_set_duty(MOTOR_L_PWM, duty);               // ����PWMռ�ձ�
}

// �����ҵ��ռ�ձȣ���ֵΪǰ������ֵΪ����
void motor_r_duty(int duty)
{
    if(duty >= 0)
    {
        gpio_set_level(MOTOR_R_DIR, GPIO_HIGH);   // ��ת����
    }
    else
    {
        gpio_set_level(MOTOR_R_DIR, GPIO_LOW);    // ��ת����
        duty = -duty;                               // ȡ����ֵ
    }
    pwm_set_duty(MOTOR_R_PWM, duty);               // ����PWMռ�ձ�
}

// ����ɢ�ȷ���ռ�ձ�
void motor_fuya_duty(int duty)
{
    pwm_set_duty(MOTOR_DUCT_PIN, duty);
}

static int duct_limit_duty(int duty)
{
    if(duty < 0) return 0;
    if(duty > DUCT_DUTY_MAX) return DUCT_DUTY_MAX;
    return duty;
}

void duct_control_update(void)
{
    int target;

    if(stop_flag && !duct_last_stop_flag)
    {
        duct_enable = 0;
    }
    duct_last_stop_flag = stop_flag;

    duct_duty = duct_limit_duty(duct_duty);
    target = duct_enable ? duct_duty : 0;

    if(duct_actual_duty < target)
    {
        duct_actual_duty += duct_ramp_step;
        if(duct_actual_duty > target) duct_actual_duty = target;
    }
    else if(duct_actual_duty > target)
    {
        duct_actual_duty -= duct_ramp_step;
        if(duct_actual_duty < target) duct_actual_duty = target;
    }

    duct_actual_duty = duct_limit_duty(duct_actual_duty);
    motor_fuya_duty(duct_actual_duty);
}

void Speed_Loop(void)
{
    float A = 0.9;  // ��ͨ�˲�ϵ��������ƽ�������
    error.currentL = (double)(motor.speedTL - encoder.speedL);   // ��ǰ��� = Ŀ���ٶ� - ʵ���ٶ�
    error.derivationL = error.currentL - 2*error.previousL + error.beforeL;  // ΢����
    error.proportionL = error.currentL - error.previousL;       // ������
    error.integrationL = error.currentL;                        // ������

    error.beforeL = error.previousL;
    error.previousL = error.currentL;
    kp_out+=(int)(kp_m*error.proportionL);
    ki_out+=(int)(ki_m*error.integrationL);
    kd_out+=(int)(kd_m*error.derivationL);

    pid_out = kp_out + ki_out + kd_out;
    motor.l_speed_out_last = motor.l_speed_out;
    motor.l_speed_out += (kp_m*error.proportionL + ki_m*error.integrationL + kd_m*error.derivationL);
    motor.l_speed_out = (A*motor.l_speed_out + (1-A)*motor.l_speed_out_last);  // ��ͨ�˲�ƽ��

    error.currentR = (double)(motor.speedTR - encoder.speedR);   // ��ǰ��� = Ŀ���ٶ� - ʵ���ٶ�
    error.derivationR = error.currentR - 2*error.previousR + error.beforeR;  // ΢����
    error.proportionR = error.currentR - error.previousR;       // ������
    error.integrationR = error.currentR;                        // ������

    error.beforeR = error.previousR;
    error.previousR = error.currentR;
    motor.r_speed_out_last = motor.r_speed_out;
    motor.r_speed_out += (kp_m*error.proportionR + ki_m*error.integrationR + kd_m*error.derivationR);
    motor.r_speed_out = (A*motor.r_speed_out + (1-A)*motor.r_speed_out_last);  // ��ͨ�˲�ƽ��

    if(motor.l_speed_out>motor_range)  motor.l_speed_out =  motor_range;
    if(motor.l_speed_out<-motor_range) motor.l_speed_out = -motor_range;
    if(motor.r_speed_out>motor_range)  motor.r_speed_out =  motor_range;
    if(motor.r_speed_out<-motor_range) motor.r_speed_out = -motor_range;

    // Steering corrections must not reverse a wheel while the car is racing.
    // Stop-state active braking remains unchanged.
    if(high_speed_forward_only && stop_flag == 0)
    {
        if(motor.speedTL >= 0 && motor.l_speed_out < 0) motor.l_speed_out = 0;
        if(motor.speedTR >= 0 && motor.r_speed_out < 0) motor.r_speed_out = 0;
    }

    motor.dutyL = (int)motor.l_speed_out;
    motor.dutyR = (int)motor.r_speed_out;

    motor_l_duty(motor.dutyL);
    motor_r_duty(motor.dutyR);
}
 
float Inner_Ring_P = 0,Inner_Ring_D=0;   
float Inner_Ring_Out = 0,Inner_Ring_Last_Out = 0;
int out_ang_speed;   // �⻷�����Ŀ����ٶ�
int Error_Current_Inner,Error_Last_Inner;
static unsigned char Inner_Ring_Ready = 0;
int Inner_Ring_PID(int expect_speed)   // �ڻ�PID�������⻷������ת��Ŀ����ٶȣ���ʵ��Z����ٶ���ƫ�ʹʵ��Z����ٶȱƽ�ת��Ŀ���ٶȣ��ú�����������ֵĲ��ٶ�
{
   float A = 0.9;  // ��ͨ�˲�ϵ��������ƽ�������
   int  ek = 0,ek1 = 0;
   Error_Current_Inner=expect_speed - post.gyro_yaw;          // ������ٶ�ƫ��
   ek=Error_Current_Inner;                                     // ��ǰƫ��
   if(Inner_Ring_Ready)
   {
       ek1=Error_Current_Inner-Error_Last_Inner;
   }
   else
   {
       ek1=0;
       Inner_Ring_Ready=1;
   }
   Error_Last_Inner=Error_Current_Inner;
   Inner_Ring_Last_Out = Inner_Ring_Out;                       // ������һ�����
   Inner_Ring_Out = Inner_Ring_P * ek + Inner_Ring_D * ek1;   // λ��ʽPD����
   Inner_Ring_Out = (int)(A*Inner_Ring_Out + (1-A)*Inner_Ring_Last_Out);  // ��ͨ�˲�ƽ��

   return (int)Inner_Ring_Out;
}

float Outer_Ring_P = 0,Outer_Ring_D = 0;  
float Outer_Ring_Out = 0,Outer_Ring_Last_Out = 0;
int Error_Last_Outer,Error_Current_Outer;
static unsigned char Outer_Ring_Ready = 0;
int Outer_Ring_PID(int expect_val,int err)   // �⻷PID��������ƫ������ת��Ϊת��Ŀ����ٶ�ֵ
{
   float A = 0.9;  // ��ͨ�˲�ϵ��������ƽ�������
   int ek = 0,ek1 = 0;
   Error_Current_Outer=err-expect_val;                             // ����ƫ��
   ek=Error_Current_Outer;                                         // ��ǰƫ��
   if(Outer_Ring_Ready)
   {
       ek1=Error_Current_Outer-Error_Last_Outer;
   }
   else
   {
       ek1=0;
       Outer_Ring_Ready=1;
   }

   // Reject derivative kick from a virtual-center discontinuity.
   if(outer_d_error_limit > 0)
   {
       if(ek1 > outer_d_error_limit) ek1 = outer_d_error_limit;
       if(ek1 < -outer_d_error_limit) ek1 = -outer_d_error_limit;
   }
   Error_Last_Outer=Error_Current_Outer;
   Outer_Ring_Last_Out = Outer_Ring_Out;                           // ������һ�����
   Outer_Ring_Out = Outer_Ring_P * ek + Outer_Ring_D * ek1;       // λ��ʽPD����
   Outer_Ring_Out = (int)(A*Outer_Ring_Out + (1-A)*Outer_Ring_Last_Out);  // ��ͨ�˲�ƽ��

   if(outer_output_limit > 0)
   {
       if(Outer_Ring_Out > outer_output_limit) Outer_Ring_Out = (float)outer_output_limit;
       if(Outer_Ring_Out < -outer_output_limit) Outer_Ring_Out = (float)(-outer_output_limit);
   }

   return (int)Outer_Ring_Out;
}

void race_request_stop(RaceStopReason_e reason)
{
    if(reason == RACE_STOP_NONE)
    {
        reason = RACE_STOP_EXTERNAL;
    }

    // Preserve the first stop cause and the course state at that exact moment.
    if(race_stop_reason == RACE_STOP_NONE)
    {
        race_stop_reason = reason;
        race_stop_node_index = Path[Path_Switch].target_angle_index;
        race_stop_distance = encoder.disA;
        race_stop_target_distance = encoder.target_distance;
        race_stop_road_type = ImageStatus.Road_type;
    }

    stop_flag = 1;
    if(race_state != RACE_STATE_STOPPED && race_state != RACE_STATE_STOPPING)
    {
        race_state = RACE_STATE_STOPPING;
        race_state_ticks = 0;
        car_depart_time = 0;
        run_to_stop_flag = 1;
        stop_to_run_flag = 0;
    }
}

void race_request_protection_stop(int offline, int pixel_left, int pixel_mid, int pixel_right)
{
    if(race_stop_reason == RACE_STOP_NONE)
    {
        race_stop_protect_black_frames = race_protect_black_frames;
        race_stop_protect_offline = offline;
        race_stop_protect_pixel_left = pixel_left;
        race_stop_protect_pixel_mid = pixel_mid;
        race_stop_protect_pixel_right = pixel_right;
    }

    race_request_stop(RACE_STOP_PROTECTION);
}

void race_start_request(void)
{
    int i;

    // Keep the ISR in a safe stopped state until every shared course state is reset.
    race_state = RACE_STATE_STOPPED;
    stop_flag = 1;

    race_stop_reason = RACE_STOP_NONE;
    race_stop_node_index = 0;
    race_stop_distance = 0;
    race_stop_target_distance = 0;
    race_stop_road_type = Normol;
    race_protect_black_frames = 0;
    race_protect_grace_ticks = 0;
    race_stop_protect_black_frames = 0;
    race_stop_protect_offline = 0;
    race_stop_protect_pixel_left = 0;
    race_stop_protect_pixel_mid = 0;
    race_stop_protect_pixel_right = 0;
    race_tick_5ms = 0;

    pid_init();
    Route_Map_Reset();
    Race_Image_Reset();
    time_pid = 0;
    car_depart_time = 0;

    for(i = 0; i < 3; i++)
    {
        Path[i].target_angle_index = 0;
    }

    encoder_reset_all();
    encoder.target_distance = Path[Path_Switch].memory_target_distance[0];

    ImageStatus.Road_type = Normol;
    ImageFlag.image_element_Node_flag = 0;
    ImageFlag.image_element_angle_flag = 0;
    ImageFlag.image_element_branch_flag = 0;
    ImageFlag.image_element_cross_flag = 0;

    post.yaw_angle = 0;
    post.target_angle = 0;
    post.diff_angle = 0;

    motor.speedTL = 0;
    motor.speedTR = 0;
    motor.speed_diff = 0;
    slow_start_speed = 0;
    motor_l_duty(0);
    motor_r_duty(0);

    duct_enable = 1;
    race_state_ticks = 0;
    stop_to_run_flag = 0;
    run_to_stop_flag = 1;

    stop_flag = 0;
    race_state = RACE_STATE_FAN_PRECHARGE;
}

static int steering_limit_center(int center)
{
    if(center < 0) return 0;
    if(center >= MT9_COMPRESS_W) return MT9_COMPRESS_W - 1;
    return center;
}

static int steering_move_towards(int current, int target, int step)
{
    if(step <= 0) return target;

    if(current < target)
    {
        current += step;
        if(current > target) current = target;
    }
    else if(current > target)
    {
        current -= step;
        if(current < target) current = target;
    }

    return current;
}

static int steering_element_direction(RoadType_e road_type)
{
    uint8_t turn;
    uint8_t index;

    if(road_type == RightAngle_L) return -1;
    if(road_type == RightAngle_R) return 1;

    if(road_type != Node_L && road_type != Node_R && road_type != Node_T
        && road_type != Branch && road_type != Cross)
    {
        return 0;
    }

    if(Path_Switch < 0 || Path_Switch >= 3) return 0;
    index = Path[Path_Switch].target_angle_index;
    if(index >= Path[Path_Switch].path_length) return 0;

    turn = Path[Path_Switch].memory_target_angle_turn[index];
    if(turn == 'L') return -1;
    if(turn == 'R') return 1;
    return 0;
}

static unsigned char steering_element_angle_running(RoadType_e road_type)
{
    if(road_type == RightAngle_L || road_type == RightAngle_R)
        return ImageFlag.image_element_angle_flag == 3;
    if(road_type == Node_L || road_type == Node_R || road_type == Node_T)
        return ImageFlag.image_element_Node_flag == 3;
    if(road_type == Branch)
        return ImageFlag.image_element_branch_flag == 3;
    if(road_type == Cross)
        return ImageFlag.image_element_cross_flag == 3;
    return 0;
}

static int steering_element_release_target(int raw_center, RoadType_e road_type)
{
    int remaining_angle;
    int exit_angle;
    int release_percent;
    int min_percent = element_release_min_percent;

    if(!steering_element_angle_running(road_type)) return raw_center;
    if(element_release_angle <= 0) return raw_center;

    remaining_angle = (post.diff_angle < 0.0) ? (int)(-post.diff_angle) : (int)post.diff_angle;
    exit_angle = angle_threshold;
    if(exit_angle < 0) exit_angle = 0;
    if(element_release_angle <= exit_angle || remaining_angle >= element_release_angle) return raw_center;

    if(min_percent < 0) min_percent = 0;
    if(min_percent > 100) min_percent = 100;

    if(remaining_angle <= exit_angle)
    {
        release_percent = min_percent;
    }
    else
    {
        release_percent = min_percent
            + ((100 - min_percent) * (remaining_angle - exit_angle))
            / (element_release_angle - exit_angle);
    }

    return ImageSensorMid + ((raw_center - ImageSensorMid) * release_percent) / 100;
}

int steering_target_update(int raw_center)
{
    RoadType_e road_type = ImageStatus.Road_type;
    int direction = steering_element_direction(road_type);
    int target_center;
    int step;
    int residual_turn_yaw;
    int remaining_angle;
    unsigned char high_yaw_release;
    unsigned char turning_element = (direction != 0);

    raw_center = steering_limit_center(raw_center);
    race_high_yaw_release_active = 0;

    if(!Steering_Target_Ready)
    {
        steering_center = raw_center;
        Steering_Target_Ready = 1;
        Steering_Element_Mode = turning_element ? 1 : 0;
        Steering_Turn_Direction = direction;
        Steering_Exit_Guard = 0;
        Steering_High_Yaw_Release_Was_Active = 0;
        return steering_center;
    }

    if(turning_element)
    {
        Steering_Element_Mode = 1;
        Steering_Turn_Direction = direction;
        Steering_Exit_Guard = 0;
        residual_turn_yaw = direction * post.gyro_yaw;
        remaining_angle = (post.diff_angle < 0.0) ? (int)(-post.diff_angle) : (int)post.diff_angle;
        high_yaw_release = (element_high_yaw_release_threshold > 0
            && element_high_yaw_release_angle > 0
            && residual_turn_yaw > element_high_yaw_release_threshold
            && remaining_angle <= element_high_yaw_release_angle);

        if(high_yaw_release)
        {
            // The remaining chassis rotation carries the final angle. Remove
            // forced steering before the element state itself is released.
            target_center = ImageSensorMid;
            step = element_high_yaw_release_step;
            if(step < element_release_step) step = element_release_step;
            race_high_yaw_release_active = 1;
        }
        else
        {
            target_center = steering_element_release_target(raw_center, road_type);
            if((direction < 0 && target_center < steering_center)
            || (direction > 0 && target_center > steering_center))
            {
                step = element_entry_step;
            }
            else
            {
                step = element_release_step;
            }
        }

        steering_center = steering_move_towards(steering_center, target_center, step);
    }
    else
    {
        if(Steering_Element_Mode == 1)
        {
            Steering_Element_Mode = 2;
            Steering_Exit_Guard = element_exit_guard_ticks;
            if(Steering_Exit_Guard < 0) Steering_Exit_Guard = 0;
        }

        if(Steering_Element_Mode == 2)
        {
            target_center = raw_center;
            residual_turn_yaw = Steering_Turn_Direction * post.gyro_yaw;

            if(element_high_yaw_release_threshold > 0
                && residual_turn_yaw > element_high_yaw_release_threshold)
            {
                // Do not add steering in the same direction while the chassis
                // is still rotating rapidly out of the completed element.
                if(Steering_Turn_Direction < 0 && target_center < ImageSensorMid)
                    target_center = ImageSensorMid;
                else if(Steering_Turn_Direction > 0 && target_center > ImageSensorMid)
                    target_center = ImageSensorMid;

                step = element_high_yaw_release_step;
                if(step < element_release_step) step = element_release_step;
                race_high_yaw_release_active = 2;
            }
            else if((Steering_Turn_Direction < 0 && target_center < steering_center)
                || (Steering_Turn_Direction > 0 && target_center > steering_center))
            {
                step = element_entry_step;
            }
            else
            {
                step = element_release_step;
            }

            steering_center = steering_move_towards(steering_center, target_center, step);

            if(Steering_Exit_Guard > 0) Steering_Exit_Guard--;
            if(Steering_Exit_Guard == 0 && steering_center == raw_center)
            {
                Steering_Element_Mode = 0;
                Steering_Turn_Direction = 0;
            }
        }
        else
        {
            steering_center = raw_center;
        }
    }

    if(race_high_yaw_release_active != 0 && !Steering_High_Yaw_Release_Was_Active)
    {
        race_high_yaw_release_count++;
    }
    Steering_High_Yaw_Release_Was_Active = (race_high_yaw_release_active != 0);

    steering_center = steering_limit_center(steering_center);
    return steering_center;
}

void pid_init(void)
{
    // �ٶȻ�PID������ʼ��
    kp_out = 0;
    ki_out = 0;
    kd_out = 0;
    motor.l_speed_out_last = 0;
    motor.l_speed_out = 0;
    motor.dutyL = 0;
    error.beforeL = 0;
    error.currentL = 0;
    error.previousL = 0;
    error.integrationL = 0;
    error.derivationL = 0;

    motor.r_speed_out_last = 0;
    motor.r_speed_out = 0;
    motor.dutyR = 0;
    error.beforeR = 0;
    error.currentR = 0;
    error.previousR = 0;
    error.integrationR = 0;
    error.derivationR = 0;

    // ���⻷PID������ʼ��
    Outer_Ring_Out = 0;
    Outer_Ring_Last_Out = 0;
    Inner_Ring_Out = 0;
    Inner_Ring_Last_Out = 0;
    Error_Current_Outer = 0;
    Error_Last_Outer = 0;
    Error_Current_Inner = 0;
    Error_Last_Inner = 0;
    Outer_Ring_Ready = 0;
    Inner_Ring_Ready = 0;
    out_ang_speed = 0;
    motor.speed_diff = 0;
    motor.speedTL = 0;
    motor.speedTR = 0;
    steering_center = ImageSensorMid;
    Steering_Target_Ready = 0;
    Steering_Element_Mode = 0;
    Steering_Turn_Direction = 0;
    Steering_Exit_Guard = 0;
    Steering_High_Yaw_Release_Was_Active = 0;
    T_Node_Speed_Active = 0;
    race_command_speed = 0;
    race_t_node_exit_hold_remaining = 0;
    race_wheel_speed_avg = 0;
    race_wheel_overshoot_percent = 0;
    race_yaw_tracking_percent = 0;
    race_slip_score = 0;
    race_high_yaw_release_active = 0;
    race_high_yaw_release_count = 0;
}

static int motor_abs_int(int value)
{
    return value < 0 ? -value : value;
}

static unsigned char motor_is_t_node_speed_target(void)
{
    if(Path_Switch != t_node_prebrake_path) return 0;
    if(Path_Switch < 0 || Path_Switch >= 3) return 0;
    if(t_node_prebrake_index < 0) return 0;
    if(Path[Path_Switch].target_angle_index != t_node_prebrake_index) return 0;
    if(Path[Path_Switch].target_angle_index >= Path[Path_Switch].path_length) return 0;
    return 1;
}

static int motor_t_node_prebrake_speed(void)
{
    long distance = encoder.disA;
    long start = t_node_prebrake_start;
    long end = t_node_prebrake_end;
    long span;
    long progress;

    if(t_node_speed < 0) return 0;
    if(end <= start || distance <= start) return target_speed;
    if(distance >= end) return t_node_speed;

    span = end - start;
    progress = distance - start;
    return target_speed - (int)(((long)(target_speed - t_node_speed) * progress) / span);
}

static void motor_update_slip_diagnostics(void)
{
    int left_speed = motor_abs_int(encoder.speedL);
    int right_speed = motor_abs_int(encoder.speedR);
    int command_speed = (motor_abs_int(motor.speedTL) + motor_abs_int(motor.speedTR)) / 2;
    int target_yaw = motor_abs_int(out_ang_speed);
    int actual_yaw = motor_abs_int(post.gyro_yaw);
    int yaw_deficit = 0;

    race_command_speed = command_speed;
    race_wheel_speed_avg = (left_speed + right_speed) / 2;
    race_wheel_overshoot_percent = 0;
    if(command_speed > 0 && race_wheel_speed_avg > command_speed)
    {
        race_wheel_overshoot_percent = ((race_wheel_speed_avg - command_speed) * 100) / command_speed;
        if(race_wheel_overshoot_percent > 999) race_wheel_overshoot_percent = 999;
    }

    race_yaw_tracking_percent = 0;
    if(target_yaw >= 500)
    {
        race_yaw_tracking_percent = (actual_yaw * 100) / target_yaw;
        if(race_yaw_tracking_percent > 999) race_yaw_tracking_percent = 999;
        if(race_yaw_tracking_percent < 100) yaw_deficit = 100 - race_yaw_tracking_percent;
    }

    race_slip_score = race_wheel_overshoot_percent + yaw_deficit;
    if(race_slip_score > 999) race_slip_score = 999;
}

// ����������Ŀ���ٶȣ�������ٶȵ��Ӳ��ٶ�ʵ�ֲ���ת��
void motor_speed_get(void)
{
    int base_speed = target_speed;
    int prebrake_speed;
    int diff_limit_percent;
    int diff_limit;
    unsigned char t_node_target = motor_is_t_node_speed_target();

    if(ImageStatus.Road_type != Normol)
    {
        base_speed = turn_speed;                  // turn/element slowdown speed
    }

    if(route_map_enable && Path_Switch == 0 && ImageStatus.Road_type == Normol)
    {
        uint8_t route_index = Path[0].target_angle_index;
        if((route_index == 11 && encoder.disA >= CM_TO_ENCODER(10))
            || (route_index == 12 && encoder.disA >= CM_TO_ENCODER(20)))
        {
            if(route_approach_speed < base_speed) base_speed = route_approach_speed;
        }
    }

    if(t_node_target)
    {
        prebrake_speed = motor_t_node_prebrake_speed();
        if(prebrake_speed < base_speed) base_speed = prebrake_speed;

        if(ImageStatus.Road_type == Node_T)
        {
            T_Node_Speed_Active = 1;
            if(t_node_speed < base_speed) base_speed = t_node_speed;
        }
    }
    else if(T_Node_Speed_Active)
    {
        T_Node_Speed_Active = 0;
        race_t_node_exit_hold_remaining = t_node_exit_hold_ticks;
        if(race_t_node_exit_hold_remaining < 0) race_t_node_exit_hold_remaining = 0;
    }

    if(!t_node_target && race_t_node_exit_hold_remaining > 0)
    {
        if(t_node_speed < base_speed) base_speed = t_node_speed;
        race_t_node_exit_hold_remaining--;
    }

    if(race_state != RACE_STATE_MOTOR_RAMP)
    {
        slow_start_speed = base_speed;
    }
    else if(slow_start_speed > base_speed)
    {
        slow_start_speed = base_speed;
    }

    // Steering authority scales with speed and cannot command wheel reversal.
    diff_limit_percent = speed_diff_limit_percent;
    if(diff_limit_percent < 0) diff_limit_percent = 0;
    if(high_speed_forward_only && diff_limit_percent > 100) diff_limit_percent = 100;
    diff_limit = (slow_start_speed * diff_limit_percent) / 100;

    if(motor.speed_diff > diff_limit) motor.speed_diff = diff_limit;
    if(motor.speed_diff < -diff_limit) motor.speed_diff = -diff_limit;

    motor.speedTL = slow_start_speed + motor.speed_diff;
    motor.speedTR = slow_start_speed - motor.speed_diff;

    if(high_speed_forward_only)
    {
        if(motor.speedTL < 0) motor.speedTL = 0;
        if(motor.speedTR < 0) motor.speedTR = 0;
    }

    motor_update_slip_diagnostics();
}

// ����ܿ��ƺ���������Ŀ���ٶȲ�ִ��PID�ٶȿ���
void motor_control(void)
{
    motor_speed_get();
    Speed_Loop();
}


