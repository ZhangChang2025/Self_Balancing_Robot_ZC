/***************************************************************************
Ҫ͢ú	LCD1602 + ؤĹǷ - ͬʱ¹ö¶¯ÎÄ×ÖºÍ²¥·ÅÒôÀÖ
Чڻú	LCD¹ö¶¯ÏÔʾÃû×Öì¬·䷄Í¬Ê±²¥·ÅСÐÇÐÇ
******************************************************************************/
#include "reg51.h"

/******** LCD IOӽڶ¨Òå ****************************************************/
sbit LCD_RS = P1^0;
sbit LCD_RW = P1^1;
sbit LCD_E  = P1^2;
#define LCD_Data P0
#define Busy    0x80

/******** ·äÃùÆ÷Òý½Å ****************************************************/
sbit SPK = P1^5;

/******** LCD ÏÔÊ¾ÎÄ×Ö ****************************************************/
unsigned char code line1[] = {"ZhangChang_2025210554    ZuoXingle_2025210561    ZhangYuxvan_2025210560    LuoBaishuo_2025210559"};
unsigned char code line2[] = {"202605_Self_Balancing_Robot"};

/******** Òô·ûÆµÊ¶¨Òå£¨Gµ÷£¬12M§ñ£© ************************************/
#define D1  0xFC,0x44   // do   262Hz
#define D2  0xFC,0xAC   // re   294Hz
#define D3  0xFD,0x09   // mi   330Hz
#define D4  0xFD,0x34   // fa   349Hz
#define D5  0xFD,0x82   // suo  392Hz
#define D6  0xFD,0xC8   // la   440Hz
#define END 0x00,0x00

/******** Ð¡ÐÇÐÇ ÀÖÆ× ****************************************************/
code unsigned int music[] = {
    D1,4,  D1,4,  D5,4,  D5,4,  D6,4,  D6,4,  D5,8,
    D4,4,  D4,4,  D3,4,  D3,4,  D2,4,  D2,4,  D1,8,
    D5,4,  D5,4,  D4,4,  D4,4,  D3,4,  D3,4,  D2,8,
    D5,4,  D5,4,  D4,4,  D4,4,  D3,4,  D3,4,  D2,8,
    D1,4,  D1,4,  D5,4,  D5,4,  D6,4,  D6,4,  D5,8,
    D4,4,  D4,4,  D3,4,  D3,4,  D2,4,  D2,4,  D1,8,
    END,0
};

/******** È«¾Ö±äÁ¿ ********************************************************/
unsigned char music_idx;     // µ±Ç°Òô·ûË÷Òý
unsigned char scroll_pos;    // ¹ö¶¯Î»ÖÃ
unsigned char str1_len;      // µÚÒ»ÐÐ×Ö·û´®³¤¶È
unsigned char str2_len;      // µÚ¶þÐÐ×Ö·û´®³¤¶È

/******** LCD º¯ÊýÉùÃ÷ ****************************************************/
void WriteDataLCD(unsigned char dat);
void WriteCommandLCD(unsigned char cmd, unsigned char check);
unsigned char ReadStatusLCD(void);
void LCDInit(void);
void LCD_ScrollOneFrame(unsigned char pos);

/******** ÑÓÊ±º¯Êý ********************************************************/
void Delay5Ms(void);
void Delay400Ms(void);
void delay(unsigned int t);

/******** Ö÷º¯Êý **********************************************************/
void main(void)
{
    unsigned char note_idx = 0;
    unsigned char beat;
    unsigned int t;

    // ³õÊ¼»¯
    Delay400Ms();
    LCDInit();
    Delay5Ms();

    // ¼ÆËã×Ö·û´®³¤¶È
    str1_len = 0;
    while(line1[str1_len] >= 0x20) str1_len++;
    str2_len = 0;
    while(line2[str2_len] >= 0x20) str2_len++;

    // ¶¨Ê±Æ÷³õÊ¼»¯£¨·äÃùÆ÷ÓÃ£©
    TMOD = 0x01;
    EA = 1;
    ET0 = 1;
    TR0 = 0;

    scroll_pos = 0;

    while(1)
    {
        // ¹ö¶¯ÏÔÊ¾Ò»Ö¡
        LCD_ScrollOneFrame(scroll_pos);

        // ²¥·Åµ±Ç°Òô·û
        if(!(music[note_idx] == 0x00 && music[note_idx+1] == 0x00))
        {
            music_idx = note_idx;
            beat = (unsigned char)music[note_idx + 2];

            TR0 = 1;
            for(t = 0; t < beat; t++)
            {
                delay(15000);
            }
            TR0 = 0;
            SPK = 1;

            note_idx += 3;
        }
        else
        {
            note_idx = 0;  // ÀÖÇú½áÊø£¬ÖØÐÂ¿ªÊ¼
        }

        // ¹ö¶¯Î»ÖÃµÝÔö
        scroll_pos++;
        if(scroll_pos > str1_len + 16 && scroll_pos > str2_len + 16)
        {
            scroll_pos = 0;
        }

        // Òô·û¼äÏ¶
        delay(3000);
        delay(3000);
    }
}

/******** ¶¨Ê±Æ÷0ÖÐ¶Ï£¨·äÃùÆ÷·¢Éù£© *************************************/
void timer0() interrupt 1
{
    TH0 = music[music_idx];
    TL0 = music[music_idx + 1];
    SPK = ~SPK;
}

/******** LCD ¹ö¶¯ÏÔÊ¾Ò»Ö¡ ***********************************************/
void LCD_ScrollOneFrame(unsigned char pos)
{
    unsigned char i;
    signed char idx;

    WriteCommandLCD(0x01, 1);  // ÇåÆÁ

    // µÚÒ»ÐÐ
    WriteCommandLCD(0x80, 0);
    for(i = 0; i < 16; i++)
    {
        idx = i + pos - 16;
        if(idx >= 0 && idx < str1_len)
            WriteDataLCD(line1[(unsigned char)idx]);
        else
            WriteDataLCD(' ');
    }

    // µÚ¶þÐÐ
    WriteCommandLCD(0xC0, 0);
    for(i = 0; i < 16; i++)
    {
        idx = i + pos - 16;
        if(idx >= 0 && idx < str2_len)
            WriteDataLCD(line2[(unsigned char)idx]);
        else
            WriteDataLCD(' ');
    }
}

/******** LCD ³õÊ¼»¯ ******************************************************/
void LCDInit(void)
{
    LCD_Data = 0;
    WriteCommandLCD(0x38,0);
    Delay5Ms();
    WriteCommandLCD(0x38,0);
    Delay5Ms();
    WriteCommandLCD(0x38,0);
    Delay5Ms();

    WriteCommandLCD(0x38,1);
    WriteCommandLCD(0x08,1);
    WriteCommandLCD(0x01,1);
    WriteCommandLCD(0x06,1);
    WriteCommandLCD(0x0C,1);
}

/******** LCD Ð´ÃüÁî ******************************************************/
void WriteCommandLCD(unsigned char cmd, unsigned char check)
{
    if(check) ReadStatusLCD();
    LCD_Data = cmd;
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_E = 0;
    LCD_E = 0;
    LCD_E = 1;
}

/******** LCD Ð´Êý¾Ý ******************************************************/
void WriteDataLCD(unsigned char dat)
{
    ReadStatusLCD();
    LCD_Data = dat;
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_E = 0;
    LCD_E = 0;
    LCD_E = 1;
}

/******** LCD ¶Á×´Ì¬ ******************************************************/
unsigned char ReadStatusLCD(void)
{
    LCD_Data = 0xFF;
    LCD_RS = 0;
    LCD_RW = 1;
    LCD_E = 0;
    LCD_E = 0;
    LCD_E = 1;
    while(LCD_Data & Busy);
    return(LCD_Data);
}

/******** ÑÓÊ±º¯Êý ********************************************************/
void Delay5Ms(void)
{
    unsigned int TempCyc = 5552;
    while(TempCyc--);
}

void Delay400Ms(void)
{
    unsigned char TempCycA = 5;
    unsigned int TempCycB;
    while(TempCycA--)
    {
        TempCycB = 7269;
        while(TempCycB--);
    }
}

void delay(unsigned int t)
{
    while(t--);
}