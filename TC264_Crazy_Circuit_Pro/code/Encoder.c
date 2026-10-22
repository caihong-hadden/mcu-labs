/*
 * Encoder.c
 *
 *  Created on: 2026��1��18��
 *      Author: Administrator
 */

#include "zf_common_headfile.h"



ENCODERINFO encoder = {0,0,0,0,0,0,0,0,0};
int current_encoder = 0;        // ��ǰ�������仯�������β���������
int encoder_sum = 0;            // �������ۼ�ֵ
int encoder_time = 0;           // ������ʱ�����
int target_distance = 0;        // Ŀ�����

void encoder_init(void)
{
    encoder_dir_init(ENCODER_DIR_L, ENCODER_DIR_PULSE_L, ENCODER_DIR_DIR_L);   // ��ʼ�������������
    encoder_dir_init(ENCODER_DIR_R, ENCODER_DIR_PULSE_R, ENCODER_DIR_DIR_R);   // ��ʼ���ұ���������
}

void encoder_get(void)
{
    encoder.speedL = -encoder_get_count(ENCODER_DIR_L);                        // ��ȡ�����ٶȣ�ȡ����������
    encoder.speedR = encoder_get_count(ENCODER_DIR_R);                         // ��ȡ�����ٶ�

    Speed_smooth_limit();                                                      // �ٶ�ƽ���޷�

    encoder_clear_count(ENCODER_DIR_L);                                        // ��������������
    encoder_clear_count(ENCODER_DIR_R);                                        // ����ұ���������
}

void Speed_smooth_limit(void)
{
    // �ٶ�ƽ���޷��߼��������������β������ٶȱ仯������20
    if(encoder.speedL > encoder.last_speedL+20.0)       encoder.speedL = encoder.last_speedL+20.0;
    else if(encoder.speedL < encoder.last_speedL-20.0)  encoder.speedL = encoder.last_speedL-20.0;

    if(encoder.speedR > encoder.last_speedR+20.0)       encoder.speedR = encoder.last_speedR+20.0;
    else if(encoder.speedR < encoder.last_speedR-20.0)  encoder.speedR = encoder.last_speedR-20.0;

    encoder.last_speedL = encoder.speedL;                                      // ���汾���ٶ���Ϊ�´�����
    encoder.last_speedR = encoder.speedR;                                      // ���汾���ٶ���Ϊ�´�����
}

void encoder_clear(void)
{
    encoder.disL = 0;
    encoder.disR = 0;
    encoder.disA = 0;
}

void encoder_reset_all(void)
{
    encoder.speedL = 0;
    encoder.speedR = 0;
    encoder.speedA = 0;
    encoder.last_speedL = 0;
    encoder.last_speedR = 0;
    encoder.disL = 0;
    encoder.disR = 0;
    encoder.disA = 0;
    encoder.target_distance = 0;
    encoder.ang_value = 0;
    encoder.ang_dir = 0;

    current_encoder = 0;
    encoder_sum = 0;
    encoder_time = 0;
    target_distance = 0;

    encoder_clear_count(ENCODER_DIR_L);
    encoder_clear_count(ENCODER_DIR_R);
}

void Dis_Measure(void)
{
    encoder.disL += encoder.speedL;                                            // �����ۼƾ���
    encoder.disR += encoder.speedR;                                            // �����ۼƾ���
    encoder.disA = (int)((encoder.disL + encoder.disR) / 2);                   // ƽ����ʻ����
    if(encoder.disA >= encoder.target_distance && Path[Path_Switch].target_angle_index >= Path[Path_Switch].path_length)    // ����ȫ���ڵ㲢�����һȦ����涨�����ͣ��
    {
        race_request_stop(RACE_STOP_COURSE_FINISH);
    }
}
