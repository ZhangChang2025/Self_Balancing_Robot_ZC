#include "bsp.h"
#include "intsever.h"

void bsp_init(void)
{
		DIY_NVIC_PriorityGroupConfig(2);	  //�����жϷ���   //Set interrupt grouping
	delay_init();	    	            //��ʱ������ʼ��	 //Delay function initialization
	JTAG_Set(JTAG_SWD_DISABLE);     //�ر�JTAG�ӿ�    //Close JTAG interface
	JTAG_Set(SWD_ENABLE);           //��SWD�ӿ� �������������SWD�ӿڵ��� //Opening the SWD interface allows for debugging using the motherboard's SWD interface
	
	//led/beep
	init_led_gpio();								//����LED  		//Onboard LED
	init_beep();										//���ط����� //Onboard buzzer
	Key1_GPIO_Init();								//���ذ���	 //Onboard buttons
	
	
	BalanceCar_Motor_Init();     	//���GPIO��ʼ��  //Motor GPIO initialization
	BalanceCar_PWM_Init(2880,0); 	//��ʼ��PWM 25Khz ����Ӳ���ӿڣ������������   Initialize PWM 25Khz and motor hardware interface for driving the motor
	Encoder_Init_TIM3();            //��ʼ��������3  Initialize encoder 3
	Encoder_Init_TIM4();            //��ʼ��������4  Initialize encoder 4
	
	uart_init(115200);	            //����1��ʼ��  Serial port 1 initialization
	bluetooth_init();								//������ʼ��  //������ʼ��   Bluetooth initialization
	
	#if K210_SWITCH
	USART2_init(115200);						//K210ͨ�Ŵ��ڳ�ʼ�� K210 communication port
	#endif
	
	delay_ms(300);
	
	IIC_MPU6050_Init();							//������i2c��ʼ��   Gyroscope I2C initialization
	MPU6050_initialize();						//���������̳�ʼ��  Gyroscope range initialization
	DMP_Init();                     //DMP��ʼ��    DMP initialization
	
	OLED_I2C_Init();							 //oled��ʼ��  OLED initialization
	
	Battery_init();									//��ص�������ʼ�� Initialization of battery level detection
	TIM2_Cap_Init(0XFFFF,72-1);    //��������ʼ��  Ultrasonic initialization
	
	//��ʱ������
	TIM6_Init();									//LED��˸����ѹ��������  //LED��˸����ѹ��������  LED flashing, voltage detection service function
	
	

}


void JTAG_Set(u8 mode)
{
	u32 temp;
	temp=mode;
	temp<<=25;
	RCC->APB2ENR|=1<<0;     //��������ʱ��	  Activate auxiliary clock  
	AFIO->MAPR&=0XF8FFFFFF; //���MAPR��[26:24] Clear MAPR [26:24]
	AFIO->MAPR|=temp;       //����jtagģʽ Set jtag mode
} 


/**************************************************************************
Function: Set NVIC group
Input   : NVIC_Group
Output  : none
�������ܣ������жϷ���
��ڲ�����NVIC_Group:NVIC���� 0~4 �ܹ�5�� 	
����  ֵ����
**************************************************************************/ 
void DIY_NVIC_PriorityGroupConfig(u8 NVIC_Group)	 
{ 
	u32 temp,temp1;	  
	temp1=(~NVIC_Group)&0x07;//ȡ����λ Take the last three
	temp1<<=8;
	temp=SCB->AIRCR;  //��ȡ��ǰ������  Read previous settings
	temp&=0X0000F8FF; //�����ǰ����   Clear previous groups
	temp|=0X05FA0000; //д��Կ��  Write the key
	temp|=temp1;	   
	SCB->AIRCR=temp;  //���÷���	  Set grouping   	  				   
}

