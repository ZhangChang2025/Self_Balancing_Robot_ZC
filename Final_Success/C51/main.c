#include "reg51.h"

sbit LCD_RS = P1^0;
sbit LCD_RW = P1^1;
sbit LCD_E  = P1^2;
#define LCD_Data P0
#define Busy 0x80

sbit KEY_CW  = P3^7;
sbit KEY_CCW = P3^6;

sbit SG90 = P1^7;
sbit SPK  = P1^5;

#define MODE_QR 'Q'
#define MODE_BT 'B'
#define MODE_ST 'S'

volatile unsigned char g_mode = MODE_ST;

unsigned char count;
unsigned char pulse_now;
unsigned char pulse_target;
unsigned char step_cnt;

void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 120; j++);
}

unsigned char ReadStatusLCD(void)
{
    LCD_Data = 0xFF;
    LCD_RS = 0;
    LCD_RW = 1;
    LCD_E = 0;
    LCD_E = 0;
    LCD_E = 1;
    while (LCD_Data & Busy);
    return LCD_Data;
}

void LCD_WriteCmd(unsigned char cmd, unsigned char check)
{
    if (check) ReadStatusLCD();
    LCD_Data = cmd;
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_E = 0;
    LCD_E = 0;
    LCD_E = 1;
}

void LCD_WriteData(unsigned char dat)
{
    ReadStatusLCD();
    LCD_Data = dat;
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_E = 0;
    LCD_E = 0;
    LCD_E = 1;
}

void LCD_Init(void)
{
    LCD_Data = 0;
    LCD_WriteCmd(0x38, 0);
    delay_ms(5);
    LCD_WriteCmd(0x38, 0);
    delay_ms(5);
    LCD_WriteCmd(0x38, 0);
    delay_ms(5);
    LCD_WriteCmd(0x38, 1);
    LCD_WriteCmd(0x08, 1);
    LCD_WriteCmd(0x01, 1);
    LCD_WriteCmd(0x06, 1);
    LCD_WriteCmd(0x0C, 1);
}

void LCD_ShowLine(unsigned char line, unsigned char code *str)
{
    unsigned char i;
    LCD_WriteCmd(line == 1 ? 0x80 : 0xC0, 1);
    for (i = 0; i < 16; i++)
    {
        if (str[i] >= 0x20)
            LCD_WriteData(str[i]);
        else
            LCD_WriteData(' ');
    }
}

void uart_init(void)
{
    TMOD &= 0x0F;
    TMOD |= 0x20;
    TH1 = 0xFD;
    TL1 = 0xFD;
    TR1 = 1;
    SCON = 0x50;
    ES = 1;
    EA = 1;
}

void set_led(unsigned char c)
{
    if (c == MODE_BT)
        P2 = 0x00;
    else if (c == MODE_QR)
        P2 = ~0x01;
    else
        P2 = ~0x04;
}

void timer0_init(void)
{
    TMOD |= 0x02;
    TH0 = 0xA4;
    TL0 = 0xA4;
    ET0 = 1;
    TR0 = 1;
}

void servo_set_target(unsigned char target)
{
    if (target > 25) target = 25;
    if (target < 5)  target = 5;
    pulse_target = target;
    step_cnt = 0;
}

void uart_isr(void) interrupt 4
{
    if (RI)
    {
        RI = 0;
        g_mode = SBUF;
        set_led(g_mode);
    }
}

void timer0_isr(void) interrupt 1
{
    if (++count >= 100)
        count = 0;

    SG90 = (count < pulse_now);

    if (pulse_now != pulse_target)
    {
        if (++step_cnt >= 20)
        {
            step_cnt = 0;
            if (pulse_now < pulse_target)
                pulse_now++;
            else
                pulse_now--;
        }
    }

    SPK = 1;
}

void main(void)
{
    uart_init();
    timer0_init();

    count = 0;
    pulse_now = 15;
    pulse_target = 15;
    step_cnt = 0;

    g_mode = MODE_ST;
    set_led(MODE_ST);

    delay_ms(300);
    LCD_Init();
    LCD_ShowLine(1, "Blue_Tooth       ");
    LCD_ShowLine(2, "                ");

    {
        unsigned char last_mode = 0;

        while (1)
        {
            if (KEY_CW == 0)
            {
                delay_ms(20);
                if (KEY_CW == 0)
                {
                    if (pulse_target <= 20)
                        servo_set_target(pulse_target + 5);
                    else
                        servo_set_target(25);
                    while (KEY_CW == 0);
                }
            }

            if (KEY_CCW == 0)
            {
                delay_ms(20);
                if (KEY_CCW == 0)
                {
                    if (pulse_target >= 10)
                        servo_set_target(pulse_target - 5);
                    else
                        servo_set_target(5);
                    while (KEY_CCW == 0);
                }
            }

            if (g_mode == MODE_QR)
            {
                if (last_mode != MODE_QR)
                {
                    last_mode = MODE_QR;
                    LCD_WriteCmd(0x01, 1);
                    LCD_ShowLine(1, "QR_Code_Control");
                    LCD_ShowLine(2, "                ");
                }
            }
            else
            {
                if (last_mode == MODE_QR)
                {
                    last_mode = g_mode;
                    LCD_WriteCmd(0x01, 1);
                    LCD_ShowLine(1, "Blue_Tooth       ");
                    LCD_ShowLine(2, "                ");
                }
            }

            delay_ms(10);
        }
    }
}
