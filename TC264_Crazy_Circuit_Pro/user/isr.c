/*********************************************************************************************************************
* TC264 Opensourec Library ����TC264 ��Դ�⣩��һ�����ڹٷ� SDK �ӿڵĵ�������Դ��
* Copyright (c) 2022 SEEKFREE ��ɿƼ�
*
* ���ļ��� TC264 ��Դ���һ����
*
* TC264 ��Դ�� ��������
* �����Ը��������������ᷢ���� GPL��GNU General Public License���� GNUͨ�ù������֤��������
* �� GPL �ĵ�3�棨�� GPL3.0������ѡ��ģ��κκ����İ汾�����·�����/���޸���
*
* ����Դ��ķ�����ϣ�����ܷ������ã�����δ�������κεı�֤
* ����û�������������Ի��ʺ��ض���;�ı�֤
* ����ϸ����μ� GPL
*
* ��Ӧ�����յ�����Դ���ͬʱ�յ�һ�� GPL �ĸ���
* ���û�У������<https://www.gnu.org/licenses/>
*
* ����ע����
* ����Դ��ʹ�� GPL3.0 ��Դ���֤Э�� �����������Ϊ���İ汾
* �������Ӣ�İ��� libraries/doc �ļ����µ� GPL3_permission_statement.txt �ļ���
* ���֤������ libraries �ļ����� �����ļ����µ� LICENSE �ļ�
* ��ӭ��λʹ�ò����������� ���޸�����ʱ���뱣����ɿƼ��İ�Ȩ����������������
*
* �ļ�����          isr
* ��˾����          �ɶ���ɿƼ����޹�˾
* �汾��Ϣ          �鿴 libraries/doc �ļ����� version �ļ� �汾˵��
* ��������          ADS v1.10.2
* ����ƽ̨          TC264D
* ��������          https://seekfree.taobao.com/
*
* �޸ļ�¼
* ����              ����                ��ע
* 2022-09-15       pudding            first version
********************************************************************************************************************/

#include "isr_config.h"
#include "isr.h"

// ����TCϵ��Ĭ���ǲ�֧���ж�Ƕ�׵ģ�ϣ��֧���ж�Ƕ����Ҫ���ж���ʹ�� interrupt_global_enable(0); �������ж�Ƕ��
// �򵥵�˵ʵ���Ͻ����жϺ�TCϵ�е�Ӳ���Զ������� interrupt_global_disable(); ���ܾ���Ӧ�κε��жϣ������Ҫ�����Լ��ֶ����� interrupt_global_enable(0); �������жϵ���Ӧ��

int stop_flag = 1;
uint8_t run_to_stop_flag = 0;
uint8_t stop_to_run_flag = 1;
int car_depart_time = 0;
int time_pid = 0;

// **************************** PIT�жϺ��� ****************************
IFX_INTERRUPT(cc60_pit_ch0_isr, 0, CCU6_0_CH0_ISR_PRIORITY)
{
    int ramp_target_speed;

    interrupt_global_enable(0);                     // �����ж�Ƕ��
    pit_clear_flag(CCU60_CH0);
    race_tick_5ms++;

    encoder_get();
    Gyro_Get();
    Gyro_Get_Angle();//Z������������
    duct_control_update();
    if(race_protect_grace_ticks > 0)
    {
        race_protect_grace_ticks--;
    }

    // Any protection or course-finish stop request joins the same controlled stop path.
    if(stop_flag != 0
        && race_state != RACE_STATE_STOPPED
        && race_state != RACE_STATE_STOPPING)
    {
        race_request_stop(RACE_STOP_EXTERNAL);
    }

    switch(race_state)
    {
        case RACE_STATE_FAN_PRECHARGE:
        {
            // Fan ramps normally, while wheels, course progress and steering history stay frozen.
            buzzer_on();
            post.yaw_angle = 0;
            motor.speedTL = 0;
            motor.speedTR = 0;
            motor.speed_diff = 0;
            slow_start_speed = 0;
            motor_l_duty(0);
            motor_r_duty(0);

            race_state_ticks++;
            car_depart_time = race_state_ticks;
            if(race_state_ticks >= RACE_FAN_PRECHARGE_TICKS)
            {
                // Remove all movement/noise accumulated while the fan was spinning up.
                encoder_reset_all();
                encoder.target_distance = Path[Path_Switch].memory_target_distance[0];
                pid_init();
                time_pid = 0;
                car_depart_time = 0;
                race_state_ticks = 0;
                buzzer_off();
                race_state = RACE_STATE_MOTOR_RAMP;
            }
            break;
        }

        case RACE_STATE_MOTOR_RAMP:
        case RACE_STATE_RUNNING:
        {
            Dis_Measure();
            if(stop_flag != 0)
            {
                race_request_stop(RACE_STOP_EXTERNAL);
                motor.speedTL = 0;
                motor.speedTR = 0;
                break;
            }

            Err = ImageSensorMid - steering_target_update(ImageStatus.Det_True);

            // Outer loop: 10 ms. Inner yaw-rate and wheel-speed loops: 5 ms.
            time_pid++;
            if(time_pid == 2)
            {
                time_pid = 0;
                out_ang_speed = -Outer_Ring_PID(0,Err);
            }
            motor.speed_diff = Inner_Ring_PID(out_ang_speed);

            if(race_state == RACE_STATE_MOTOR_RAMP)
            {
                ramp_target_speed = target_speed;
                if(ImageStatus.Road_type != Normol)
                {
                    ramp_target_speed = turn_speed;
                }

                race_state_ticks++;
                slow_start_speed = (ramp_target_speed * race_state_ticks) / RACE_MOTOR_RAMP_TICKS;
                car_depart_time = race_state_ticks;
                if(race_state_ticks >= RACE_MOTOR_RAMP_TICKS)
                {
                    slow_start_speed = ramp_target_speed;
                    race_state = RACE_STATE_RUNNING;
                }
            }

            motor_control();
            break;
        }

        case RACE_STATE_STOPPING:
        {
            motor.speedTL = 0;
            motor.speedTR = 0;
            motor.speed_diff = 0;
            Speed_Loop();

            race_state_ticks++;
            car_depart_time = race_state_ticks;
            if(race_state_ticks > Total_Time)
            {
                stop_to_run_flag = 1;
                run_to_stop_flag = 0;
                race_state = RACE_STATE_STOPPED;
            }
            break;
        }

        case RACE_STATE_STOPPED:
        default:
        {
            motor.speedTL = 0;
            motor.speedTR = 0;
            motor.speed_diff = 0;
            slow_start_speed = 0;
            motor_l_duty(0);
            motor_r_duty(0);
            break;
        }
    }
}

IFX_INTERRUPT(cc60_pit_ch1_isr, 0, CCU6_0_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��
    pit_clear_flag(CCU60_CH1);

}

IFX_INTERRUPT(cc61_pit_ch0_isr, 0, CCU6_1_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��
    pit_clear_flag(CCU61_CH0);




}

IFX_INTERRUPT(cc61_pit_ch1_isr, 0, CCU6_1_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��
    pit_clear_flag(CCU61_CH1);




}
// **************************** PIT�жϺ��� ****************************


// **************************** �ⲿ�жϺ��� ****************************
IFX_INTERRUPT(exti_ch0_ch4_isr, 0, EXTI_CH0_CH4_INT_PRIO)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��
    if(exti_flag_get(ERU_CH4_REQ8_P33_7))           // ͨ��4�ж�
    {
        exti_flag_clear(ERU_CH4_REQ8_P33_7);
        camera_vsync_handler_1();                   // ����ͷ1 �����ɼ�ͳһ�ص�����
    }

    if(exti_flag_get(ERU_CH0_REQ0_P15_4))           // ͨ��0�ж�
    {
        exti_flag_clear(ERU_CH0_REQ0_P15_4);
        // ע��: ���ⲿ�ж�Ϊ����ͷ���ж� ����ʹ������ͷ �벻Ҫ�������������
        // ע��: ���ⲿ�ж�Ϊ����ͷ���ж� ����ʹ������ͷ �벻Ҫ�������������
        // ע��: ���ⲿ�ж�Ϊ����ͷ���ж� ����ʹ������ͷ �벻Ҫ�������������




    }

}

// ��������ͷpclk����Ĭ��ռ���� 1ͨ�������ڴ���DMA��������ﲻ�ٶ����жϺ���
IFX_INTERRUPT(exti_ch1_ch5_isr, 0, EXTI_CH1_CH5_INT_PRIO)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��

    if(exti_flag_get(ERU_CH1_REQ10_P14_3))          // ͨ��1�ж�
    {
        exti_flag_clear(ERU_CH1_REQ10_P14_3);
        // ע��: ���ⲿ�ж�Ϊ����ͷPCLK ����ʹ������ͷ �벻Ҫ�������������
        // ע��: ���ⲿ�ж�Ϊ����ͷPCLK ����ʹ������ͷ �벻Ҫ�������������
        // ע��: ���ⲿ�ж�Ϊ����ͷPCLK ����ʹ������ͷ �벻Ҫ�������������




    }

    if(exti_flag_get(ERU_CH5_REQ1_P15_8))           // ͨ��5�ж�
    {
        exti_flag_clear(ERU_CH5_REQ1_P15_8);
        // ע��: ���ⲿ�ж�Ϊ����ͷPCLK ����ʹ������ͷ �벻Ҫ�������������
        // ע��: ���ⲿ�ж�Ϊ����ͷPCLK ����ʹ������ͷ �벻Ҫ�������������
        // ע��: ���ⲿ�ж�Ϊ����ͷPCLK ����ʹ������ͷ �벻Ҫ�������������




    }
}

IFX_INTERRUPT(exti_ch2_ch6_isr, 0, EXTI_CH2_CH6_INT_PRIO)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��
    if(exti_flag_get(ERU_CH2_REQ7_P00_4))           // ͨ��2�ж�
    {
        exti_flag_clear(ERU_CH2_REQ7_P00_4);
        // ע��: ���ⲿ�ж�Ϊ����ͷPCLK ����ʹ������ͷ �벻Ҫ�������������
        // ע��: ���ⲿ�ж�Ϊ����ͷPCLK ����ʹ������ͷ �벻Ҫ�������������
        // ע��: ���ⲿ�ж�Ϊ����ͷPCLK ����ʹ������ͷ �벻Ҫ�������������



    }
    if(exti_flag_get(ERU_CH6_REQ9_P20_0))           // ͨ��6�ж�
    {
        exti_flag_clear(ERU_CH6_REQ9_P20_0);
        // ע��: ���ⲿ�ж�Ϊ����ͷPCLK ����ʹ������ͷ �벻Ҫ�������������
        // ע��: ���ⲿ�ж�Ϊ����ͷPCLK ����ʹ������ͷ �벻Ҫ�������������
        // ע��: ���ⲿ�ж�Ϊ����ͷPCLK ����ʹ������ͷ �벻Ҫ�������������




    }
}

IFX_INTERRUPT(exti_ch3_ch7_isr, 0, EXTI_CH3_CH7_INT_PRIO)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��

    if(exti_flag_get(ERU_CH3_REQ3_P10_3))           // ͨ��3�ж�
    {
        exti_flag_clear(ERU_CH3_REQ3_P10_3);
        camera_vsync_handler_2();                   // ����ͷ2�����ɼ�ͳһ�ص�����
    }

    if(exti_flag_get(ERU_CH7_REQ16_P15_1))          // ͨ��7�ж�
    {
        exti_flag_clear(ERU_CH7_REQ16_P15_1);
        // ע��: ���ⲿ�ж�Ϊ����ͷ���ж� ����ʹ������ͷ �벻Ҫ�������������
        // ע��: ���ⲿ�ж�Ϊ����ͷ���ж� ����ʹ������ͷ �벻Ҫ�������������
        // ע��: ���ⲿ�ж�Ϊ����ͷ���ж� ����ʹ������ͷ �벻Ҫ�������������



    }
}
// **************************** �ⲿ�жϺ��� ****************************


// **************************** DMA�жϺ��� ****************************
IFX_INTERRUPT(dma_ch6_isr, 0, DMA_INT_PRIO_1)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��
    camera_dma_handler_1();                         // ����ͷ1 �ɼ����ͳһ�ص�����
}

IFX_INTERRUPT(dma_ch7_isr, 0, DMA_INT_PRIO_2)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��
    camera_dma_handler_2();                         // ����ͷ2 �ɼ����ͳһ�ص�����
}
// **************************** DMA�жϺ��� ****************************


// **************************** �����жϺ��� ****************************
// ����0Ĭ����Ϊ���Դ���
IFX_INTERRUPT(uart0_tx_isr, 0, UART0_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��



}
IFX_INTERRUPT(uart0_rx_isr, 0, UART0_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��

#if DEBUG_UART_USE_INTERRUPT                        // ������� debug �����ж�
        debug_interrupr_handler();                  // ���� debug ���ڽ��մ������ ���ݻᱻ debug ���λ�������ȡ
#endif                                              // ����޸��� DEBUG_UART_INDEX ����δ�����Ҫ�ŵ���Ӧ�Ĵ����ж�ȥ
}


// ����1Ĭ�����ӵ�����ͷ���ô���
IFX_INTERRUPT(uart1_tx_isr, 0, UART1_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��




}
IFX_INTERRUPT(uart1_rx_isr, 0, UART1_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��
    camera_uart_handler_1();                        // ����ͷ��������ͳһ�ص�����
}

// ����2Ĭ�����ӵ�����ת����ģ��
IFX_INTERRUPT(uart2_tx_isr, 0, UART2_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��



}

IFX_INTERRUPT(uart2_rx_isr, 0, UART2_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��
    wireless_module_uart_handler();                 // ����ģ��ͳһ�ص�����



}
// ����3Ĭ�����ӵ�GPS��λģ��
IFX_INTERRUPT(uart3_tx_isr, 0, UART3_TX_INT_PRIO)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��



}

IFX_INTERRUPT(uart3_rx_isr, 0, UART3_RX_INT_PRIO)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��

    gnss_uart_callback();                           // GNSS���ڻص�����



}

// ����ͨѶ�����ж�
IFX_INTERRUPT(uart0_er_isr, 0, UART0_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��
    IfxAsclin_Asc_isrError(&uart0_handle);
}
IFX_INTERRUPT(uart1_er_isr, 0, UART1_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��
    IfxAsclin_Asc_isrError(&uart1_handle);
}
IFX_INTERRUPT(uart2_er_isr, 0, UART2_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��
    IfxAsclin_Asc_isrError(&uart2_handle);
}
IFX_INTERRUPT(uart3_er_isr, 0, UART3_ER_INT_PRIO)
{
    interrupt_global_enable(0);                     // �����ж�Ƕ��
    IfxAsclin_Asc_isrError(&uart3_handle);
}
