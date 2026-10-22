/*
 * Encoder.h
 *
 *  Created on: 2026��1��18��
 *      Author: Administrator
 */

#include "zf_common_headfile.h"

#ifndef CODE_ENCODER_H_
#define CODE_ENCODER_H_

#define ENCODER_DIR_L                   (TIM4_ENCODER)                         // ���������Ӧ�ı������ӿ�
#define ENCODER_DIR_PULSE_L             (TIM4_ENCODER_CH1_P02_8)               // ������� PULSE ��Ӧ����
#define ENCODER_DIR_DIR_L               (TIM4_ENCODER_CH2_P00_9)               // ������� DIR ��Ӧ����

#define ENCODER_DIR_R                   (TIM6_ENCODER)                         // �ұ�������Ӧ�ı������ӿ�
#define ENCODER_DIR_PULSE_R             (TIM6_ENCODER_CH1_P20_3)               // �ұ����� PULSE ��Ӧ����
#define ENCODER_DIR_DIR_R               (TIM6_ENCODER_CH2_P20_0)               // �ұ����� DIR ��Ӧ����

#define CM_TO_ENCODER(x)             (x * 316)                                // ���׵������������ת��ϵ������λcm

typedef struct
{
    int speedL;                        // �����ٶ�
    int speedR;                        // �����ٶ�
    int speedA;                        // ƽ���ٶ�

    int last_speedL;                   // ��һ�������ٶȣ�����ƽ���޷���
    int last_speedR;                   // ��һ�������ٶȣ�����ƽ���޷���

    long disL;                         // �����ۼƾ���
    long disR;                         // �����ۼƾ���
    long disA;                         // ƽ����ʻ����
    long target_distance;              // Ŀ�����

    int ang_value;                     // �Ƕ�ֵ
    int ang_dir;                       // �Ƕȷ���
}ENCODERINFO;

extern ENCODERINFO encoder;            // ����������ʵ��
extern int current_encoder;            // ��ǰ�������仯�������β���������

void encoder_init(void);               // ��������ʼ��
void encoder_get(void);                // ��ȡ�������ٶ�
void Dis_Measure(void);                // ����������ۼ�
void encoder_clear(void);              // ������ʻ����
void encoder_reset_all(void);          // 复位编码器全部状态
void Speed_smooth_limit(void);         // �ٶ�ƽ���޷�


#endif /* CODE_ENCODER_H_ */
