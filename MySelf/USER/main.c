/**
* @par Copyright (C): 2018-2028, Shenzhen Yahboom Tech
* @file         // main.c
* @author       // lly
* @version      // V1.0
* @date         // 240628
* @brief        // ������� Program entry
* @details      
* @par History  // �޸���ʷ��¼�б���ÿ���޸ļ�¼Ӧ�����޸����ڡ��޸��߼�
*               // �޸����ݼ���  Modification history list, each modification record should include the modification date, modifier and a brief description of the modification content
*/ 

#include "AllHeader.h"
#include "intsever.h"
//ע��:������������ʱ��Ҫ�ж��Ƿ���������ѹ
//Attention: When operating the buzzer, check if it is at normal voltage

uint8_t GET_Angle_Way=2;                             //��ȡ�Ƕȵ��㷨��1����Ԫ��  2��������  3�������˲�  //Algorithm for obtaining angles, 1: Quaternion 2: Kalman 3: Complementary filtering
float Angle_Balance,Gyro_Balance,Gyro_Turn;     		//ƽ����� ƽ�������� ת�������� //Balance tilt angle balance gyroscope steering gyroscope
int Motor_Left,Motor_Right;                 	  		//���PWM���� //Motor PWM variable
int Temperature;                                		//�¶ȱ��� 		//Temperature variable
float Acceleration_Z;                           		//Z����ٶȼ�  //Z-axis accelerometer
int Voltage,Mid_Angle;                          		//��ص�ѹ������صı�������е��ֵ Battery voltage sampling related variables, mechanical median
float Move_X,Move_Z; //Move_X:ǰ���ٶ�  Move_Z��ת���ٶ�  //Move_X: Forward speed Move_Z: Steering speed
u8 Stop_Flag = 1; //0:��ʼ 1:ֹͣ  //0: Start 1: Stop

u8 weight_mode_flag = 0;	//����ģʽ 0:�ر� 1:����  //Load mode 0: disabled 1: enabled
enCarMode g_car_mode = enMODE_READY;	//С������ģʽ Car mode

char showbuf[20]={'\0'};

extern u8 newLineReceived;
extern u8 bulettohflag;

int main(void)
{	
	Mid_Angle = 1; //����С������ȡ //Obtain based on the car
	
	
	bsp_init();
	
	MPU6050_EXTI_Init();					//���жϷ������ŵ���� //This interrupt service function is placed last
	
	OLED_Draw_Line("put down key start!", 1, true, true); 

	// READY״̬: ��KEY1���뵽ң��ģʽ Ready state: press KEY1 to enter Bluetooth mode
	while(!Key1_State(1));
	delay_ms(50);	//ȥ���� Debounce
	Stop_Flag = 0; //��ʼ���� Start controlling
	g_car_mode = enMODE_BLUETOOTH;
	OLED_Draw_Line("Bluetooth Mode!", 1, true, true); 
	BEEP_BEEP = 1;
	delay_ms(50);
	BEEP_BEEP = 0;

	while(1)
	{
		// KEY1�л�ģʽ Bluetooth��Weightģʽ�л� KEY1 toggle between Bluetooth and Weight mode
		if(Key1_State(1))
		{
			delay_ms(50);
			if(g_car_mode == enMODE_BLUETOOTH)
			{
				g_car_mode = enMODE_WEIGHT;
				weight_mode_flag = 1;
				OLED_Draw_Line("Weight Mode!   ", 1, true, true);
			}
			else
			{
				g_car_mode = enMODE_BLUETOOTH;
				weight_mode_flag = 0;
				OLED_Draw_Line("Bluetooth Mode!", 1, true, true);
			}
			BEEP_BEEP = 1;
			delay_ms(50);
			BEEP_BEEP = 0;
			
			sprintf(showbuf,"weight_mode = %d  ",weight_mode_flag);
			OLED_Draw_Line(showbuf, 2, false, true);
		}
		
		if (newLineReceived) //����ң�ط��� Bluetooth remote control service
		{
			ProtocolCpyData();
			Protocol();
		}
		if(bulettohflag == 1) //�˷����ϱ������ݣ�app�����bug The data reported by this method may cause a bug in the app
		{
			bulettohflag = 0;
			SendAutoUp();//�����Զ��ϱ����� Bluetooth automatically reports data 
		}
		
		
		sprintf(showbuf,"angle = %.2f  ",Angle_Balance);
		OLED_Draw_Line(showbuf, 3, false, true); 
	
	}
}


