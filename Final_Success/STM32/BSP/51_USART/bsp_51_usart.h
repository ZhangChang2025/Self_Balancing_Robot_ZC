#ifndef __BSP_51_USART_H_
#define __BSP_51_USART_H_

#include "stm32f10x.h"

#define USART3_51_BAUD 10417

void USART3_51_Init(void);
void USART3_51_SendByte(uint8_t ch);
uint8_t USART3_51_GetCmd(void);

#endif
