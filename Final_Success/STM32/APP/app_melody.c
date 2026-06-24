#include "app_melody.h"
#include "bsp_beep.h"
#include "bsp_timer.h"

#define E3  165
#define G3  196
#define A3  220
#define B3  247
#define C4  262
#define D4  294
#define E4  330
#define F4  349
#define G4  392
#define A4  440
#define B4  494
#define C5  523
#define D5  587
#define E5  659
#define F5  698
#define G5  784
#define A5  880
#define B5  988
#define C6  1047
#define RST 0

typedef struct {
    uint16_t freq;
    uint8_t beat;
} Note_t;

static const Note_t twinkle[] = {
    {C4,4},{C4,4},{G4,4},{G4,4},{A4,4},{A4,4},{G4,8},
    {F4,4},{F4,4},{E4,4},{E4,4},{D4,4},{D4,4},{C4,8},
    {G4,4},{G4,4},{F4,4},{F4,4},{E4,4},{E4,4},{D4,8},
    {G4,4},{G4,4},{F4,4},{F4,4},{E4,4},{E4,4},{D4,8},
    {C4,4},{C4,4},{G4,4},{G4,4},{A4,4},{A4,4},{G4,8},
    {F4,4},{F4,4},{E4,4},{E4,4},{D4,4},{D4,4},{C4,8},
    {RST,0}
};

static const Note_t mario[] = {
    {E4,2},{E4,2},{RST,2},{E4,2},{RST,2},{C4,2},{E4,4},
    {G4,4},{RST,4},{G3,4},{RST,4},
    {C4,2},{RST,2},{G3,2},{RST,2},{E3,4},
    {RST,2},{A3,2},{B3,2},{RST,2},{A3,2},{RST,2},{G3,2},
    {E4,2},{G4,2},{A4,4},{F4,2},{G4,2},{RST,2},{E4,2},{C4,2},{D4,2},{B3,4},
    {RST,4},
    {C4,2},{RST,2},{G3,2},{RST,2},{E3,4},
    {RST,2},{A3,2},{B3,2},{RST,2},{A3,2},{RST,2},{G3,2},
    {E4,2},{G4,2},{A4,4},{F4,2},{G4,2},{RST,2},{E4,2},{C4,2},{D4,2},{B3,4},
    {RST,4},
    {RST,0}
};

static void TIM7_Start(uint16_t hz)
{
    if (hz == 0)
    {
        TIM_Cmd(TIM7, DISABLE);
        BEEP_BEEP = 0;
        return;
    }
    uint16_t arr = 1000000 / (2 * hz);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
    TIM7->PSC = 71;
    TIM7->ARR = arr;
    TIM7->CNT = 0;
    TIM7->SR = 0;
    TIM7->DIER = TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM7_IRQn);
    TIM_Cmd(TIM7, ENABLE);
}

static void TIM7_Stop(void)
{
    TIM_Cmd(TIM7, DISABLE);
    NVIC_DisableIRQ(TIM7_IRQn);
    BEEP_BEEP = 0;
}

static void play_note(uint16_t freq, uint8_t beat)
{
    TIM7_Start(freq);
    delay_time(beat * 3);
    TIM7_Stop();
}

static void play_song(const Note_t *song)
{
    for (int i = 0; song[i].beat > 0; i++)
        play_note(song[i].freq, song[i].beat);
}

void Melody_Play_Twinkle(void)
{
    play_song(twinkle);
}

void Melody_Play_Mario(void)
{
    play_song(mario);
}

void TIM7_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
        BEEP_BEEP = !BEEP_BEEP;
    }
}
