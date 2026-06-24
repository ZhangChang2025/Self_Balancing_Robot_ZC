/**
* @par Copyright (C): 2018-2028, Shenzhen Yahboom Tech
* @file         // main.c
* @author       // lly
* @version      // V1.0
* @date         // 240628
* @brief        // Program entry
* @details      
* @par History  //
*               
*/ 

#include "AllHeader.h"
#include "intsever.h"

uint8_t GET_Angle_Way=2;
float Angle_Balance,Gyro_Balance,Gyro_Turn;
int Motor_Left,Motor_Right;
int Temperature;
float Acceleration_Z;
int Voltage,Mid_Angle;
float Move_X,Move_Z;
u8 Stop_Flag = 1;

u8 weight_mode_flag = 0;
enCarMode g_car_mode = enMODE_READY;

char showbuf[20]={'\0'};

extern u8 newLineReceived;
extern u8 bulettohflag;

int main(void)
{	
	Mid_Angle = 1;
	
	bsp_init();
	
	MPU6050_EXTI_Init();
	
	OLED_Draw_Line("put down key start!", 1, true, true); 

	while(!Key1_State(1));
	delay_ms(50);
	Stop_Flag = 0;
	g_car_mode = enMODE_BLUETOOTH;
	OLED_Draw_Line("Bluetooth Mode!", 1, true, true); 
	BEEP_BEEP = 1;
	delay_ms(50);
	BEEP_BEEP = 0;

	while(1)
	{
		if(Key1_State(1))
		{
			delay_ms(50);
			if(g_car_mode == enMODE_BLUETOOTH)
			{
				g_car_mode = enMODE_QR;
				OLED_Draw_Line("QR_Code_Control", 1, true, true);
			}
			else
			{
				g_car_mode = enMODE_BLUETOOTH;
				OLED_Draw_Line("Bluetooth Mode!", 1, true, true);
			}
			BEEP_BEEP = 1;
			delay_ms(50);
			BEEP_BEEP = 0;
		}
		
		if(g_car_mode == enMODE_QR)
		{
			Change_state();
		}

		{
			static enCarMode s_last_51_mode = enMODE_READY;
			if (s_last_51_mode != g_car_mode)
			{
				s_last_51_mode = g_car_mode;
				if (g_car_mode == enMODE_QR)
					USART3_51_SendByte('Q');
				else if (g_car_mode == enMODE_BLUETOOTH)
					USART3_51_SendByte('B');
				else
					USART3_51_SendByte('S');
			}
		}
		
		if (newLineReceived)
		{
			ProtocolCpyData();
			Protocol();
		}
		if(bulettohflag == 1)
		{
			bulettohflag = 0;
			SendAutoUp();
		}

		sprintf(showbuf,"angle = %.2f  ",Angle_Balance);
		OLED_Draw_Line(showbuf, 3, false, true);
	}
}
