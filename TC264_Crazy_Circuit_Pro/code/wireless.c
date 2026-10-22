/*
 * wireless.c
 *
 *  ���ߴ��ڵ���ģ�� �� ͨ�����ߴ���ʵ��Vofa+��λ��������ʾ�����߲�������
 *
 *  Created on: 2026��1��22��
 *      Author: Administrator
 */

#include "zf_common_headfile.h"

uint8_t Wireless_Receive_Data = 0;   // ���ߴ��ڽ��յ��İ������ݣ�1~21��Ӧ��ͬ����ָ�
uint8_t Motor_Runing_Flag = 0;       // ������б�־λ��?=ֹͣ��1=����
uint8_t Vofa_Show_Model = 0;         // Vofa+��ʾģʽѡ��0=�ٶȻ�������1=�⻷������2=�ڻ�������3=�ۺ���Ϣ��
uint8_t Wireless_init_flag = 1;      // ����ģ���ʼ����־λ��?=δ��ʼ����1=�ѳ�ʼ��

static char Vofa_Send_Buffer[512];

// Vofa+��λ��������ʾ����
// ����Vofa_Show_Model��ֵ������ǰPID�������ٶȡ����ٶȵȵ�����Ϣͨ�����ߴ��ڷ��͵���λ��
void Vofa_Show(void)
{
    switch(Vofa_Show_Model)
    {
        case 0: // ģʽ0���ٶȻ�������ʾ
        {   // ��ʾ������ʵ���ٶȡ�Ŀ���ٶȡ����ռ�ձȡ���?�ڻ�PID���������PID��������ǰ·���Ƕ�����
            wifi_spi_send_buffer((const uint8 *)Vofa_Send_Buffer, (uint32)sprintf(Vofa_Send_Buffer, "%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%ld,%ld,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%lu\r\n",encoder.speedL,target_speed,encoder.speedR,motor.dutyL,motor.speedTL,out_ang_speed,post.gyro_yaw,Err,Outer_Ring_P,Outer_Ring_D,Inner_Ring_P,Inner_Ring_D,kp_m,ki_m,kd_m,Path[Path_Switch].target_angle_index,turn_speed,duct_duty,duct_actual_duty,duct_enable,TowPoint,ImageStatus.TowPoint,race_state,race_stop_reason,Path_Switch,Path[Path_Switch].target_angle_index,Path[Path_Switch].path_length,encoder.disA,encoder.target_distance,ImageStatus.Road_type,race_protect_black_frames,race_stop_node_index,race_stop_distance,race_stop_target_distance,race_stop_road_type,ImageStatus.OFFLine,race_protect_grace_ticks,race_stop_protect_black_frames,race_stop_protect_offline,race_stop_protect_pixel_left,race_stop_protect_pixel_mid,race_stop_protect_pixel_right,race_command_speed,race_t_node_exit_hold_remaining,race_wheel_speed_avg,race_wheel_overshoot_percent,race_yaw_tracking_percent,race_slip_score,route_expected_road_type,route_raw_road_type,route_fallback_active,route_fallback_count,route_fallback_distance,ImageStatus.First_W_Point_L,ImageStatus.First_W_Point_R,ImageFrameDetect.spinodal_L_Ysite,ImageFrameDetect.spinodal_R_Ysite,route_rearm_active,route_rearm_frames,race_high_yaw_release_active,race_high_yaw_release_count,(unsigned long)race_tick_5ms));
        }   break;
        case 1: // ģʽ1���⻷������ʾ
        {   // ��ʾ�������ٶȡ����ռ�ձȡ��⻷���ٶȡ�������Yaw�ǡ��ٶȲƫ���Լ��⻷/�ڻ�PD����
            wifi_spi_send_buffer((const uint8 *)Vofa_Send_Buffer, (uint32)sprintf(Vofa_Send_Buffer, "%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,%f\r\n",encoder.speedL,encoder.speedR,motor.dutyL,motor.dutyR,out_ang_speed,post.gyro_yaw,motor.speed_diff,Err,Outer_Ring_P,Outer_Ring_D,Inner_Ring_P,Inner_Ring_D));
        }   break;
        case 2: // ģʽ2���ڻ�������ʾ
        {  // ��ʾ�ڻ�PDֵ���⻷������ٶȡ�������Z����ٶȡ��ٶȲ�?            wifi_spi_send_buffer((const uint8 *)Vofa_Send_Buffer, (uint32)sprintf(Vofa_Send_Buffer, "%f,%f,%d,%d,%d\r\n",Inner_Ring_P,Inner_Ring_D,out_ang_speed,post.gyro_yaw,motor.speed_diff));    // ����ڻ�PDֵ���⻷���ٶ�
        }   break;
        case 3: // ģʽ3���ۺ���Ϣ��ʾ
        {   // ��ʾ�⻷/�ڻ�PD������Ŀ���ٶȡ���·���͡�ƫ�ǰհ����Ϣ
            wifi_spi_send_buffer((const uint8 *)Vofa_Send_Buffer, (uint32)sprintf(Vofa_Send_Buffer, "%f,%f,%f,%f,%f,%f,%f,%d,%d,%d,%d,%d,%d,%d,%d\r\n",Outer_Ring_P,Outer_Ring_D,Inner_Ring_P,Inner_Ring_D,kp_m,ki_m,kd_m,target_speed,ImageStatus.Road_type,Err,TowPoint,ImageStatus.TowPoint,duct_duty,duct_actual_duty,duct_enable));
        }   break;
        default:
            break;
    }
}

// Vofa+���߲�����������
// ͨ�����ߴ��ڽ��հ���ָ�1~21����ʵʱ�޸�PID������Ŀ���ٶȡ����ٶȵ����в���
void Vofa_Adjust_Parameter(void)
{
    Race_Image_Transmit_Update();
    Wireless_Receive_Data = 0;// ÿ�ζ�ȡ�󽫲������㣬��������ָ��ʱ�ظ�ִ����һ�β���
//    if(wireless_uart_read_buffer(&Wireless_Receive_Data, 1) == 0)
//          {
//              return;
//          }
   // wireless_uart_read_buffer(&Wireless_Receive_Data,1);// �����ߴ��ڻ�������ȡָ�����ݵ�Wireless_Receive_Data
    while(wifi_spi_read_buffer(&Wireless_Receive_Data, 1) != 0)
  {
    switch (Wireless_Receive_Data)
    {
        case 1:  // ���Kp+0.1
        {
            kp_m += 0.1;
        }   break;
        case 2:  // ���Kp-0.1
        {
            kp_m -= 0.1;
        }   break;
        case 3:  // ���Ki+0.01
        {
            ki_m+=0.01;
        }   break;
        case 4:  // ���Ki-0.01
        {
            ki_m-=0.01;
        }   break;
        case 5:  // ���Kd+0.1
        {
            kd_m += 0.1;
        }   break;
        case 6:  // ���Kd-0.1
        {
            kd_m -= 0.1;
        }   break;
        case 7:  // Ŀ���ٶ�+30
        {
            target_speed += 10;
        }   break;
        case 8:  // Ŀ���ٶ�-30
        {
            target_speed -= 10;
        }   break;
        case 9:  // ��ͣ�л�
        {
            if(stop_flag == 0)
            {
                race_request_stop(RACE_STOP_MANUAL);
            }
            else
            {
                race_start_request();
            }
        }   break;
        case 10: // �⻷P+5
        {
            Outer_Ring_P +=5;
        }   break;
        case 11: // �⻷P-5
        {
            Outer_Ring_P -=5;
        }   break;
        case 12: // �⻷D+5
        {
            Outer_Ring_D +=5;
        }   break;
        case 13: // �⻷D-5
        {
            Outer_Ring_D -=5;
        }   break;
        case 14: // �ڻ�P+0.001
        {
            Inner_Ring_P += 0.001;
        }   break;
        case 15: // �ڻ�P-0.001
        {
            Inner_Ring_P -= 0.001;
        }   break;
        case 16: // �ڻ�D+0.001
        {
            Inner_Ring_D += 0.001;
        }   break;
        case 17: // �ڻ�D-0.001
        {
            Inner_Ring_D -= 0.001;
        }   break;
        case 18: // �⻷���ٶ�+500
        {
            out_ang_speed += 500;
        }   break;
        case 19: // �⻷���ٶ�-500
        {
            out_ang_speed -= 500;
        }   break;
        case 20: // ����ǰ����д��EEPROM����
        {
            flash_write_mydata();
        }   break;
        case 21: // ��ȡͼ��ÿ�п�ȣ������ã�?
        {
            for(int i = 0;i<60;i++)
            {
                printf("��%d�п��?%d������\r\n",i,ImageDeal[i].Wide);
            }
        }   break;
        case 22: // ��λ�����·���
          {
              race_start_request();
          }
          break;

          case 23: // ǿ��ͣ��
          {
              race_request_stop(RACE_STOP_MANUAL);

              motor.speedTL = 0;
              motor.speedTR = 0;
              motor_l_duty(0);
              motor_r_duty(0);
              // duct is controlled independently by VOFA open/close.
          }
          break;
          case 24: // 0x18 ·��һ
            {
                Path_Switch = 0;
                LED_on(Path_Switch);
            }
            break;

            case 25: // 0x19 ·����
            {
                Path_Switch = 1;
                LED_on(Path_Switch);
            }
            break;

            case 26: // 0x1A ·����
            {
                Path_Switch = 2;
                LED_on(Path_Switch);
            }
            break;
        case 27: // speed Kp fine +0.01
        {
            kp_m += 0.01;
        }   break;
        case 28: // speed Kp fine -0.01
        {
            kp_m -= 0.01;
        }   break;
        case 29: // speed Ki fine +0.001
        {
            ki_m += 0.001;
        }   break;
        case 30: // speed Ki fine -0.001
        {
            ki_m -= 0.001;
        }   break;
        case 31: // speed Kd fine +0.01
        {
            kd_m += 0.01;
        }   break;
        case 32: // speed Kd fine -0.01
        {
            kd_m -= 0.01;
        }   break;
        case 33: // outer P fine +1
        {
            Outer_Ring_P += 1;
        }   break;
        case 34: // outer P fine -1
        {
            Outer_Ring_P -= 1;
        }   break;
        case 35: // outer D fine +1
        {
            Outer_Ring_D += 1;
        }   break;
        case 36: // outer D fine -1
        {
            Outer_Ring_D -= 1;
        }   break;
        case 37: // inner P fine +0.0001
        {
            Inner_Ring_P += 0.0001;
        }   break;
        case 38: // inner P fine -0.0001
        {
            Inner_Ring_P -= 0.0001;
        }   break;
        case 39: // inner D fine +0.0001
        {
            Inner_Ring_D += 0.0001;
        }   break;
        case 40: // inner D fine -0.0001
        {
            Inner_Ring_D -= 0.0001;
        }   break;
        case 41: // yaw target speed fine +100
        {
            out_ang_speed += 100;
        }   break;
        case 42: // yaw target speed fine -100
        {
            out_ang_speed -= 100;
        }   break;
        case 43: // target speed fine +1
        {
            target_speed += 1;
        }   break;
        case 44: // target speed fine -1
        {
            target_speed -= 1;
        }   break;
        case 47: // TowPoint +1, look closer
        {
            ImageStatus.TowPoint += 1;
            if(ImageStatus.TowPoint > 48) ImageStatus.TowPoint = 48;
        }   break;
        case 48: // TowPoint -1, look farther
        {
            ImageStatus.TowPoint -= 1;
            if(ImageStatus.TowPoint < 12) ImageStatus.TowPoint = 12;
        }   break;
        case 49: // turn speed +5
        {
            turn_speed += 5;
        }   break;
        case 50: // turn speed -5
        {
            turn_speed -= 5;
            if(turn_speed < 0) turn_speed = 0;
        }   break;
        case 51: // duct target +500
        {
            duct_duty += 500;
            if(duct_duty > DUCT_DUTY_MAX) duct_duty = DUCT_DUTY_MAX;
        }   break;
        case 52: // duct target -500
        {
            duct_duty -= 500;
            if(duct_duty < 0) duct_duty = 0;
        }   break;
        case 53: // duct target +100
        {
            duct_duty += 100;
            if(duct_duty > DUCT_DUTY_MAX) duct_duty = DUCT_DUTY_MAX;
        }   break;
        case 54: // duct target -100
        {
            duct_duty -= 100;
            if(duct_duty < 0) duct_duty = 0;
        }   break;
        case 55: // duct open, ramp to target
        {
            duct_enable = 1;
        }   break;
        case 56: // duct close, ramp down to 0
        {
            duct_enable = 0;
        }   break;
        default:
            break;
    }
}
}
