#include "bsp_LED.h"


void Init_Led_GPIO(void)
{
		
		GPIO_InitTypeDef GPIO_InitStructure;
	
	
		//初始化LED的引脚 Initialize the LED pins
		RCC_APB2PeriphClockCmd(LED_RCC, ENABLE); 									   
  	GPIO_InitStructure.GPIO_Pin = LED_PIN;	
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
  	GPIO_Init(LED_PORT, &GPIO_InitStructure);
	
		LED = 0;

}


