/**
* @par Copyright (C): 2016-2026, Shenzhen Yahboom Tech
* @file         // ALLHeader.h
* @author       // lly
* @version      // V1.0
* @date         // 240628
* @brief        // ������е�ͷ�ļ� All related header files
* @details      
* @par History  //
*               
*/


#ifndef __ALLHEADER_H
#define __ALLHEADER_H


//ͷ�ļ� Header Files
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>


#include "stm32f10x.h"
#include "stm32f10x_gpio.h"


#include "switch_function.h"
#include "myenum.h"

#include "delay.h"
#include "bsp.h"
#include "bsp_battery.h"
#include "bsp_beep.h"
#include "bsp_LED.h"
#include "bsp_timer.h"
#include "bsp_key.h"

//Usart
#include "usart.h"	

//51 MCU communication (USART3)
#include "bsp_51_usart.h"

//K210
#include "bsp_usart2.h"
#include "app_k210.h"


//���� Bluetooth
#include "bsp_bluetooth.h"
#include "app_bluetooth.h"

//OLED
#include "bsp_oled.h"
#include "bsp_oled_i2c.h"

//Mpu6050
#include "IOI2C.h"
#include "MPU6050.h"
#include "dmpKey.h"
#include "dmpmap.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"

//Motor
#include "motor.h"
#include "encoder.h"
#include "app_motor.h"

//������ ultrasound
#include "bsp_ultrasonic.h"

//ƽ�⳵������� Balance car overall control
#include "app_control.h"
#include "pid_control.h"

//filtering alforithm
#include "filter.h"
#include "KF.h"



//������ͨ�ñ��� General variables introduced
extern float Velocity_Left,Velocity_Right; 								//���ӵ��ٶ� The speed of the wheels
extern uint8_t GET_Angle_Way;                             //��ȡ�Ƕȵ��㷨��1����Ԫ��  2��������  3�������˲�  Algorithm for obtaining angles, 1: Quaternion 2: Kalman 3: Complementary filtering
extern float Angle_Balance,Gyro_Balance,Gyro_Turn;     		//ƽ����� ƽ�������� ת�������� Balance tilt angle balance gyroscope steering gyroscope
extern int Motor_Left,Motor_Right;                 	  		//���PWM���� Motor PWM variable
extern int Temperature;                                		//�¶ȱ��� Temperature variable
extern float Acceleration_Z;                           		//Z����ٶȼ�  Z-axis accelerometer
extern int 	Mid_Angle;                          				//��е��ֵ  Mechanical median
extern float Move_X,Move_Z;															//Move_X:ǰ���ٶȣ�Forward speed��  Move_Z��ת���ٶ�(Turning speed)
extern float battery; 																	//��ص���	battery level 
extern u8 lower_power_flag; 														//�͵�ѹ��־,��ѹ�ָ���־ Low voltage sign, voltage recovery sign
extern u32 g_distance; 																	//����������ֵ Ultrasonic distance value
extern u8 Flag_velocity; 																//�ٶȿ�����ر��� Speed control related variables
extern enCarState g_newcarstate; 												//С��״̬��־ Car status indicator
extern u8 Stop_Flag;																		//ֹͣ��־  Stop sign
extern u8 weight_mode_flag;																//���ر�־  Weight-bearing flag
extern enCarMode g_car_mode;															//С������ģʽ  Car mode
#endif


