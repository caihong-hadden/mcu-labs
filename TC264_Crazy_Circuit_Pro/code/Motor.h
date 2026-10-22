/*
 * Motor.h
 *
 *  Created on: 2026��1��18��
 *      Author: Administrator
 */

#ifndef CODE_MOTOR_H_
#define CODE_MOTOR_H_

// ������Ŷ��壨���İ� / �ɰ����ã������ã�
 #define MOTOR_L_PWM                         (ATOM3_CH5_P11_10)                   // ������Ӧ��PWMͨ��
 #define MOTOR_R_PWM                         (ATOM2_CH7_P11_12)                   // �ҵ����Ӧ��PWMͨ��
 #define MOTOR_L_DIR                         (P11_9)                         // ������Ӧ�ķ���ӿ�
 #define MOTOR_R_DIR                         (P11_11)                          // �ҵ����Ӧ�ķ���ӿ�

//#define MOTOR_DIR_L (P11_9)
//#define MOTOR_PWM_L (ATOM3_CH5_P11_10)
//#define MOTOR_DIR_R (P11_11)
//#define MOTOR_PWM_R (ATOM2_CH7_P11_12)
//�װ�һ��
//#define MOTOR_L_PWM                         (ATOM3_CH6_P11_11)                   // ������Ӧ��PWMͨ��
//#define MOTOR_R_PWM                         (ATOM3_CH4_P11_9)                   // �ҵ����Ӧ��PWMͨ��
//#define MOTOR_L_DIR                         ( ATOM3_CH5_P11_10)                         // ������Ӧ�ķ���ӿ� ԭ���İ嶨��: ATOM3_CH5_P11_10
//#define MOTOR_R_DIR                         (ATOM3_CH7_P11_12)                          // �ҵ����Ӧ�ķ���ӿ� ԭ���İ嶨��: ATOM3_CH7_P11_12

#define MOTOR_DUCT_PIN                      (ATOM1_CH0_P21_2)                         // ���ȵ��ʹ�õ�����
#define DUCT_DUTY_MAX                      (8000)                         // max duct pwm duty

typedef enum
{
    RACE_STATE_STOPPED = 0,
    RACE_STATE_FAN_PRECHARGE,
    RACE_STATE_MOTOR_RAMP,
    RACE_STATE_RUNNING,
    RACE_STATE_STOPPING
} RaceState_e;

typedef enum
{
    RACE_STOP_NONE = 0,
    RACE_STOP_MANUAL,
    RACE_STOP_COURSE_FINISH,
    RACE_STOP_PROTECTION,
    RACE_STOP_EXTERNAL
} RaceStopReason_e;

#define RACE_FAN_PRECHARGE_TICKS           (400) // 400 * 5 ms = 2 s
#define RACE_MOTOR_RAMP_TICKS               (200) // 200 * 5 ms = 1 s
#define RACE_PROTECT_EXIT_GRACE_TICKS         (8) // 8 * 5 ms = 40 ms after element exit

typedef struct
{
    int dutyL;                  // ����PWMռ�ձ����
    int dutyR;                  // �ҵ��PWMռ�ձ����
    float l_speed_out;          // ����PID�ٶȻ����
    float r_speed_out;          // �ҵ��PID�ٶȻ����
    float l_speed_out_last;     // ������һ��PID��������ڵ�ͨ�˲���
    float r_speed_out_last;     // �ҵ����һ��PID��������ڵ�ͨ�˲���

    int speed_diff;             // �����ֲ��ٶȣ����ڲ���ת��
    int speedD;                 // �ٶ�D����
    int speedTL;                // ����Ŀ���ٶ�
    int speedTR;                // ����Ŀ���ٶ�
}MOTOR;

typedef struct
{
    double currentL;            // ���ֵ�ǰ��= Ŀ���ٶ� - ʵ���ٶȣ�
    double currentR;            // ���ֵ�ǰ��= Ŀ���ٶ� - ʵ���ٶȣ�
    double previousL;           // ������һ�����
    double previousR;           // ������һ�����
    double beforeL;             // �������ϴ����
    double beforeR;             // �������ϴ����
    double derivationL;         // ����΢���= ��ǰ��� - 2*��һ����� + ���ϴ���
    double derivationR;         // ����΢���= ��ǰ��� - 2*��һ����� + ���ϴ���
    double integrationL;        // ���ֻ����= ��ǰ��
    double integrationR;        // ���ֻ����= ��ǰ��
    double proportionL;         // ���ֱ����= ��ǰ��� - ��һ����
    double proportionR;         // ���ֱ����= ��ǰ��� - ��һ����
}ERROR;

extern MOTOR motor;                // ���״̬�ṹ��
extern ERROR error;                // ���ṹ��
extern float kp_m;                 // �ٶȻ�����ϵ��
extern float ki_m;                 // �ٶȻ�����ϵ��
extern float kd_m;                 // �ٶȻ�΢��ϵ��
extern int ki_out;                 // ���������
extern int kp_out;                 // ���������
extern int kd_out;                 // ΢�������
extern int pid_out;                // PID�����
extern int motor_range;            // PWM����޷�ֵ
extern int out_ang_speed;          // �⻷�����Ŀ����ٶ�
extern int target_speed;           // Ŀ���ٶ�
extern int slow_start_speed;       // ������ٶ�
extern int turn_speed;             // turn/element base speed
extern int outer_d_error_limit;    // max outer-loop error step used by D term
extern int outer_output_limit;     // max absolute target yaw-rate command
extern int speed_diff_limit_percent; // max differential command as percent of base speed
extern int high_speed_forward_only;  // forbid wheel reverse while the car is running
extern int element_entry_step;       // virtual-center attack step per 5ms control tick
extern int element_release_step;     // virtual-center release step per 5ms control tick
extern int element_release_angle;    // begin releasing forced steering below this angle
extern int element_release_min_percent; // minimum forced steering authority near element exit
extern int element_exit_guard_ticks; // minimum exit-shaping time in 5ms ticks
extern int element_high_yaw_release_threshold; // residual yaw threshold for fast exit release
extern int element_high_yaw_release_step; // fast virtual-center release step
extern int element_high_yaw_release_angle; // remaining angle that enables pre-exit release
extern int steering_center;           // center actually used by the steering outer loop
extern float Outer_Ring_P;         // �⻷��ƫ�������ϵ��
extern float Outer_Ring_D;         // �⻷��ƫ���΢��ϵ��
extern float Inner_Ring_P;         // �ڻ������ٶȻ�������ϵ��
extern float Inner_Ring_D;         // �ڻ������ٶȻ���΢��ϵ��
extern int duct_duty;              // duct target duty
extern int duct_actual_duty;       // duct ramp output duty
extern int duct_enable;            // duct switch, independent from wheel start
extern int duct_ramp_step;         // duct soft-start step per tick
extern volatile RaceState_e race_state;
extern volatile int race_state_ticks;
extern volatile uint32_t race_tick_5ms;
extern volatile RaceStopReason_e race_stop_reason;
extern volatile int race_stop_node_index;
extern volatile int race_stop_distance;
extern volatile int race_stop_target_distance;
extern volatile int race_stop_road_type;
extern volatile int race_protect_black_frames;
extern volatile int race_protect_grace_ticks;
extern volatile int race_stop_protect_black_frames;
extern volatile int race_stop_protect_offline;
extern volatile int race_stop_protect_pixel_left;
extern volatile int race_stop_protect_pixel_mid;
extern volatile int race_stop_protect_pixel_right;


extern int t_node_prebrake_path;
extern int t_node_prebrake_index;
extern int t_node_prebrake_start;
extern int t_node_prebrake_end;
extern int t_node_speed;
extern int t_node_exit_hold_ticks;
extern volatile int race_command_speed;
extern volatile int race_t_node_exit_hold_remaining;
extern volatile int race_wheel_speed_avg;
extern volatile int race_wheel_overshoot_percent;
extern volatile int race_yaw_tracking_percent;
extern volatile int race_slip_score;
extern volatile int race_high_yaw_release_active; // 0=off, 1=before exit, 2=after exit
extern volatile int race_high_yaw_release_count;

void motor_init(void);
void motor_l_duty(int duty);        // ��������ռ�ձ�
void motor_r_duty(int duty);        // �����ҵ��ռ�ձ�
void motor_fuya_duty(int duty);     // ���÷���ռ�ձ�
void duct_control_update(void);     // independent duct soft-start control
void race_start_request(void);      // reset the full course and start fan precharge
void race_request_stop(RaceStopReason_e reason); // controlled stop with first-cause snapshot
void race_request_protection_stop(int offline, int pixel_left, int pixel_mid, int pixel_right);
void Speed_Loop(void);              // �ٶȻ�PID���ƣ�����ʽ��
void motor_control(void);           // ����ܿ��ƺ���
int Outer_Ring_PID(int expect_val,int err);  // �⻷PID��ƫ� �� Ŀ����ٶȣ�
int Inner_Ring_PID(int expect_speed);        // �ڻ�PID�����ٶȻ� �� ���ٶȣ�
int steering_target_update(int raw_center);  // race-mode element entry/exit target shaping
void motor_speed_get(void);         // ����������Ŀ���ٶ�
void pid_init(void);                // PID������ʼ��

#endif /* CODE_MOTOR_H_ */

