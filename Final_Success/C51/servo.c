#include "reg51.h"

#define STEP_DELAY 40
#define STEP 5

sbit KEY1 = P3^7;
sbit KEY2 = P3^6;
sbit SG90 = P1^7;

unsigned char count;
unsigned char pulse_now;
unsigned char pulse_target;
unsigned char step_cnt;

void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for(i = ms; i > 0; i--)
        for(j = 250; j > 0; j--);
}

void main()
{
    SG90 = 0;

    TMOD = 0x01;
    TH0 = 0xFF;
    TL0 = 0x9C;
    EA = 1;
    ET0 = 1;
    TR0 = 1;

    pulse_now = 5;
    pulse_target = 5;
    count = 0;
    step_cnt = 0;

    while(1)
    {
        if(KEY1 == 0)
        {
            delay_ms(40);
            if(KEY1 == 0)
            {
                if(pulse_now <= 20)
                    pulse_target = pulse_now + STEP;
                else
                    pulse_target = 25;
                step_cnt = 0;
                while(KEY1 == 0);
            }
        }
        if(KEY2 == 0)
        {
            delay_ms(40);
            if(KEY2 == 0)
            {
                if(pulse_now >= 10)
                    pulse_target = pulse_now - STEP;
                else
                    pulse_target = 5;
                step_cnt = 0;
                while(KEY2 == 0);
            }
        }
    }
}

void timer0() interrupt 1
{
    TH0 = 0xFF;
    TL0 = 0x9C;

    if(++count >= 200)
        count = 0;

    SG90 = (count < pulse_now);

    if(pulse_now != pulse_target)
    {
        if(++step_cnt >= STEP_DELAY)
        {
            step_cnt = 0;
            if(pulse_now < pulse_target)
                pulse_now++;
            else
                pulse_now--;
        }
    }
}
