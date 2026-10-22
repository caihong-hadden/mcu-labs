/*
 * eeprom.c
 *
 *  EEPROM�����洢ģ�� �� �����õĲ����־û�д��Flash������ʱ��Flash��ȡ�ָ�����
 *
 *  Created on: 2026��4��2��
 *      Author: Administrator
 */

#include "zf_common_headfile.h"

uint8_t flash_flag = 0; // EEPROM������Ч��־λ��0=δд���/��Ч��1=��д����Ч����

// ���ݳ�ʼ������
// ��EEPROM��ȡ������ȫ�ֱ�������EEPROM������Ч���ݣ�flash_flag==0������ʹ��Ĭ�ϲ���ֵ
void data_init(void)
{
    flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);  // ��EEPROM��ȡһҳ���ݵ�������
    flash_flag = flash_union_buffer[0].uint8_type;                     // �ӻ�����[0]��ȡ������Ч��־

    if(flash_flag == 1)
    {
        // EEPROM������Ч���ݣ��ӻ�������λ�ö�ȡ������ȫ�ֱ���
        Outer_Ring_P = flash_union_buffer[1].float_type;               // ������[1]���⻷P
        Outer_Ring_D = flash_union_buffer[2].float_type;               // ������[2]���⻷D
        Inner_Ring_P = flash_union_buffer[3].float_type;               // ������[3]���ڻ�P
        Inner_Ring_D = flash_union_buffer[4].float_type;               // ������[4]���ڻ�D
        kp_m = flash_union_buffer[5].float_type;                       // ������[5]�����Kp
        ki_m = flash_union_buffer[6].float_type;                       // ������[6]�����Ki
        kd_m = flash_union_buffer[24].float_type;                      // ������[24]�����Kd
        target_speed = flash_union_buffer[7].uint16_type;              // ������[7]��Ŀ���ٶ�
        duct_duty = flash_union_buffer[8].uint16_type;                 // ������[8]�����ռ�ձ�
        ImageStatus.TowPoint = flash_union_buffer[9].uint16_type;      // ������[9]��ǰհ��
        Element_lock.RightAngle_L = flash_union_buffer[10].uint8_type; // ������[10]����ֱ�Ǳ�־
        Element_lock.RightAngle_R = flash_union_buffer[11].uint8_type; // ������[11]����ֱ�Ǳ�־
        Element_lock.Node_L = flash_union_buffer[12].uint8_type;       // ������[12]�������־
        Element_lock.Node_R = flash_union_buffer[13].uint8_type;       // ������[13]���ҽ���־
        Element_lock.Node_T = flash_union_buffer[14].uint8_type;       // ������[14]��T�ͽ���־
        Element_lock.Branch = flash_union_buffer[15].uint8_type;       // ������[15]����֧·��־
        Element_lock.Cross = flash_union_buffer[16].uint8_type;        // ������[16]��ʮ��·�ڱ�־
        gamma_value = flash_union_buffer[17].float_type;               // ������[17]��GammaУ��ֵ
        OSTU_High = flash_union_buffer[18].uint16_type;                // ������[18]�������ֵ����
        OSTU_Low = flash_union_buffer[19].uint16_type;                 // ������[19]�������ֵ����
        distance_judgment = flash_union_buffer[20].float_type;         // ������[20]�������ж���ֵ
        Path_Switch = flash_union_buffer[21].uint16_type;              // ������[21]��·���л�ѡ��
        motor_range = flash_union_buffer[22].uint16_type;              // ������[22]�����PWM��Χ
        Element_lock.Protect = flash_union_buffer[23].uint8_type;      // ������[23]��������־
    }
    else if(flash_flag == 0)
    {
        // EEPROM��û����Ч���ݣ�ʹ��Ĭ�ϲ���ֵ
        Outer_Ring_P = 350;
        Outer_Ring_D = 100;
        Inner_Ring_P = 0.066;
        Inner_Ring_D = 0.036;
        kp_m = 6.6;
        ki_m = 0.4;
        kd_m = 0;
        target_speed = 450;
        duct_duty = 500;
        ImageStatus.TowPoint = 16;
        Element_lock.RightAngle_L = 1;   // Ĭ�ϣ�������ֱ�Ǽ��
        Element_lock.RightAngle_R = 1;   // Ĭ�ϣ�������ֱ�Ǽ��
        Element_lock.Node_L = 1;         // Ĭ�ϣ�����������
        Element_lock.Node_R = 1;         // Ĭ�ϣ������ҽ����
        Element_lock.Node_T = 1;         // Ĭ�ϣ�����T�ͽ����
        Element_lock.Branch = 1;         // Ĭ�ϣ����÷�֧·���
        Element_lock.Cross = 1;          // Ĭ�ϣ�����ʮ��·�ڼ��
        gamma_value = 0.5;
        OSTU_High = 150;
        OSTU_Low = 10;
        distance_judgment = 0;
        Path_Switch = 0;
        motor_range = 8888;
        Element_lock.Protect = 1;
    }
}

// ����ǰ����д��EEPROM�־û��洢
// �Ƚ�ȫ�ֱ���ֵ���뻺������Ȼ�����Ŀ��ҳ����󽫻�����д��Flash
void flash_write_mydata(void)
{
    flash_erase_page(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);           // ����Ŀ��Flashҳ

    flash_union_buffer[0].uint8_type = 1;                              // ������[0]��д����Ч��־=1
    flash_union_buffer[1].float_type = Outer_Ring_P;                   // ������[1]���⻷P
    flash_union_buffer[2].float_type = Outer_Ring_D;                   // ������[2]���⻷D
    flash_union_buffer[3].float_type = Inner_Ring_P;                   // ������[3]���ڻ�P
    flash_union_buffer[4].float_type = Inner_Ring_D;                   // ������[4]���ڻ�D
    flash_union_buffer[5].float_type = kp_m;                           // ������[5]�����Kp
    flash_union_buffer[6].float_type = ki_m;                           // ������[6]�����Ki
    flash_union_buffer[7].uint16_type = target_speed;                  // ������[7]��Ŀ���ٶ�
    flash_union_buffer[8].uint16_type = duct_duty;                     // ������[8]�����ռ�ձ�
    flash_union_buffer[9].uint16_type = ImageStatus.TowPoint;          // ������[9]��ǰհ��
    flash_union_buffer[10].uint8_type = Element_lock.RightAngle_L;     // ������[10]����ֱ�Ǳ�־
    flash_union_buffer[11].uint8_type = Element_lock.RightAngle_R;     // ������[11]����ֱ�Ǳ�־
    flash_union_buffer[12].uint8_type = Element_lock.Node_L;           // ������[12]�������־
    flash_union_buffer[13].uint8_type = Element_lock.Node_R;           // ������[13]���ҽ���־
    flash_union_buffer[14].uint8_type = Element_lock.Node_T;           // ������[14]��T�ͽ���־
    flash_union_buffer[15].uint8_type = Element_lock.Branch;           // ������[15]����֧·��־
    flash_union_buffer[16].uint8_type = Element_lock.Cross;            // ������[16]��ʮ��·�ڱ�־
    flash_union_buffer[17].float_type = gamma_value;                   // ������[17]��GammaУ��ֵ
    flash_union_buffer[18].uint16_type = OSTU_High;                    // ������[18]�������ֵ����
    flash_union_buffer[19].uint16_type = OSTU_Low;                     // ������[19]�������ֵ����
    flash_union_buffer[20].float_type = distance_judgment;             // ������[20]�������ж���ֵ
    flash_union_buffer[21].uint16_type = Path_Switch;                  // ������[21]��·���л�ѡ��
    flash_union_buffer[22].uint16_type = motor_range;                  // ������[22]�����PWM��Χ
    flash_union_buffer[23].uint8_type = Element_lock.Protect;          // ������[23]��������־
    flash_union_buffer[24].float_type = kd_m;                          // ������[24]�����Kd

    flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX); // ������������д��FlashĿ��ҳ

    buzzer_on();           // д����ɺ��������ʾ
    system_delay_ms(100);
    buzzer_off();
}