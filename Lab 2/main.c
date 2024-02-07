/************** ECE2049 DEMO CODE ******************/
/**************  13 March 2019   ******************/
/***************************************************/


#include <msp430.h>
#include <stdio.h>
#include "peripherals.h"

long unsigned int timer_cnt=0;
long unsigned int prev_time=0;
char tdir = 1;
int SongNote = 0;
uint8_t led;
uint8_t button;
int flag = 0;

void runtimerA2(void);
void stoptimerA2(int reset);
__interrupt void TimerA2_ISR (void);
//void updateLCD(char currentDisplay[], char string[]);

void configUserLED(char inbits);
void configUserButtons(void);
uint8_t getState(void);



enum GAME_STATE { WELCOME = 0, COUNTDOWN = 1, PLAY_NOTE = 2, CHECK_NOTE = 3, GAME_OVER = 4, YOU_WIN = 5, EXIT = 6};
GAME_STATE = GAME_OVER;

struct song {
    unsigned int beats[50];
    unsigned int frequency[50];
    };


unsigned char currKey;
int main(void) {
    char currentDisplay[] = "new";
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
    _BIS_SR(GIE); //enables interrupts
    runtimerA2(); //start timer

    initLeds();
    configDisplay();
    configKeypad();

    int ticksPerSec = 100;
    int LEDbit = 0;

    unsigned char currKey = getKey();
    unsigned char currKeyint = getKey();

    struct song miiSong = {
      {1  ,1, 1,  1, 1,  1, 1,  1,  1,  1, 4, 1,  1,  1,  1,  1, 1, 1, 1, 1,  3,  1,  1, 1,2, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1,  1, 1,2, 1,  1, 1,  1,2, 2,  2,  1, 1, 1,  1, 1,  1,1, 1, 1,   1,  1, 1, 1,  1,  1, 1,1, 1,  1,  1, 1, 1, 1, 1,  4, 1,  2,1, 1,  1,  1, 2,  1,  1,   1, 1,   1, 1,   2,  1,  1,  1,  1,  1,  1, 1},
      {370,0,440,554,0,440,370,294,294,294,0,208,294,370,440,550,0,440,0,370,659,622,587,0,0,415,0,554,370,0,554,0,415,0,554,0,392,370,0,330,0,262,262,262,0,0,277,277,261,0,0,311,294,370,0,440,554,0,440,0,370,294,294,294,0,659,659,659,0,0,370,440,554,0,440,0,370,659,587,0,0,493,392,293,262,493,415,293,440,370,263,247,349,293,247,230,230,230}
    };

    while(1){
        int my_state = GAME_STATE;
        currKey = getKey();
        char currKeyint = getKey();
        switch(GAME_STATE){
            case WELCOME: //display Welcome Screen
               setLeds(0);
               Graphics_drawStringCentered(&g_sContext, "Guitar Hero", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
               Graphics_flushBuffer(&g_sContext);
               if (currKey == 42)
               {
                   GAME_STATE = COUNTDOWN;
                   Graphics_clearDisplay(&g_sContext); // Clear the display
               }
               else
               {
                   GAME_STATE = WELCOME;
               }
               prev_time = timer_cnt;
               break;
            case COUNTDOWN: //counts down
                currKeyint = (getKey() - 48);
                currKey = getKey();
                Graphics_flushBuffer(&g_sContext);
                //prev_time = timer_cnt;
                 if (currKey == 35) //#
                 {
                     GAME_STATE = EXIT;
                     break;
                 }
                 if ((timer_cnt - prev_time) > (500)){
                     setLeds(0);
                     Graphics_clearDisplay(&g_sContext); // Clear the display
                     prev_time = timer_cnt;
                     GAME_STATE = PLAY_NOTE;
                 }
                 else if ((timer_cnt - prev_time) > (400)){
                     Graphics_drawStringCentered(&g_sContext, "GO!!!", AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);
                     setLeds(BIT3|BIT2|BIT1|BIT0);
                 }
                 else if((timer_cnt - prev_time) > (3*100)){
                     Graphics_drawStringCentered(&g_sContext, "  1  ", AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);
                     setLeds(BIT2);
                 }
                 else if ((timer_cnt - prev_time) > (2*100)){
                     Graphics_drawStringCentered(&g_sContext, "  2  ", AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);
                     setLeds(BIT1);
                 }
                 else if ((timer_cnt - prev_time) > (1*100)){
                     Graphics_drawStringCentered(&g_sContext, "  3  ", AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);
                     setLeds(BIT0);
                 }
                 if ((GAME_STATE != EXIT)&&(GAME_STATE != PLAY_NOTE)){
                     GAME_STATE = COUNTDOWN;
                 }
                 break;
            case PLAY_NOTE:
                currKey = getKey();
                if (currKey == 35) //#
                {
                    GAME_STATE = EXIT;
                    break;
                }

                //play the note via a function & flash an LED
                //BuzzerOn(miiSong.frequency[SongNote]);
//                setLeds(BIT1); //change later
                if((timer_cnt - prev_time) <= miiSong.beats[SongNote]*100){ //40
//                  calculate which button and led to light and press
                    BuzzerOn(miiSong.frequency[SongNote]);
                    if ((miiSong.frequency[SongNote] >= 208)&&(miiSong.frequency[SongNote] < 321))
                    {
                        led = BIT0;
                    }
                    else if ((miiSong.frequency[SongNote] > 321)&&(miiSong.frequency[SongNote] < 434))
                    {
                        led = BIT1;
                    }
                    else if ((miiSong.frequency[SongNote] > 434)&&(miiSong.frequency[SongNote] < 547))
                    {
                        led = BIT2;
                    }
                    else if ((miiSong.frequency[SongNote] > 547)&&(miiSong.frequency[SongNote] <= 659))
                    {
                        led = BIT3;
                    }
                    setLeds(led);
                    button |= getState();
                    //check for button pressed

                }else{
                    if (led != button)
                    {
                        GAME_STATE = YOU_WIN;
                    }
                    BuzzerOff();
                    SongNote++;
                    prev_time = timer_cnt;
                }

                //start playing note for the duration of beats
                //while playing check for button inputs
                    //if button played is correct keep going
                    //if incorrect button is played, imediatly end
                //onec duration is over check weather correct button was played, if yes, increment to next note
                break;
            case CHECK_NOTE:
                break;
            case GAME_OVER:
                setLeds(getState());
                break;
            case YOU_WIN:
                BuzzerOn(440);
                break;
            case EXIT:
                setLeds(0);
                Graphics_clearDisplay(&g_sContext); // Clear the display
                GAME_STATE = WELCOME;
                break;
        }
    }
}
void runtimerA2(void){
// This function configures and starts Timer A2
// Timer is counting ~0.01 seconds
//
// Input: none, Output: none
//
// smj, ECE2049, 17 Sep 2013
//
// Use ACLK, 16 Bit, up mode, 1 divider
    TA2CTL = TASSEL_1 + MC_1 + ID_0;
    TA2CCR0 = 327; // 327+1 = 328 ACLK tics = ~1/100 seconds
    TA2CCTL0 = CCIE; // TA2CCR0 interrupt enabled
}
void stoptimerA2(int reset)
{
// This function stops Timer A2 andresets the global time variable
// if input reset = 1
//
// Input: reset, Output: none
//
// smj, ECE2049, 17 Sep 2013
//
    TA2CTL = MC_0; // stop timer
    TA2CCTL0 &= ~CCIE; // TA2CCR0 interrupt disabled
        if(reset)
            timer_cnt=0;
}
// Timer A2 interrupt service routine
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TimerA2_ISR (void){
    if(tdir){
        timer_cnt++;
//        if (timer_cnt == 60000)
//            timer_cnt = 0;
//        if (timer_cnt%100==0){ // blink LEDs once a second
//            P1OUT = P1OUT ^ BIT0;
//            P4OUT ^= BIT7;
//        }
    }
    else
        timer_cnt--;
}

#pragma vector=TIMER2_A1_VECTOR
__interrupt void TimerA1_ISR (void){
    if(currKey == 35){
        flag = 1;
        GAME_STATE = EXIT;
        TA1CCTL0 &= ~CCIFG;
    }
    else
        TA1CCTL0 &= ~CCIFG;
}

void configUserLED(char inbits){
    P1SEL &= (BIT0);
    P4SEL &= (BIT7);

    P1DIR |= (BIT0);
    P4DIR |= (BIT7);

    P1OUT = inbits &= BIT0;
    P2OUT = inbits &= BIT1;

}
void configUserButtons(void){
    P7SEL &= (BIT0|BIT4); //S1, S4
    P3SEL &= (BIT7); //S2
    P2SEL &- (BIT2); //S3

    P7DIR &= (BIT0|BIT4); //S1, S4
    P3DIR &= (BIT7); //S2
    P2DIR &- (BIT2); //S3

    P7REN |= (BIT0|BIT4); //S1, S4
    P3REN |= (BIT7); //S2
    P2REN |= (BIT2); //S3\

    P7OUT |= (BIT0|BIT4); //S1, S4
    P3OUT |= (BIT7); //S2
    P2OUT |= (BIT2); //S3

    //Configure # to be an interrupt pin

}
uint8_t getState(void){
        uint8_t result = 0x00;
        if (~P7IN & BIT0) {
            result = BIT3;
        }
        if (~P3IN & BIT7) {
            result = BIT2;
        }
        if (~P2IN & BIT2) {
            result = BIT1;
        }
        if (~P7IN & BIT4) { //bit 4 is set
            result = BIT0;
        }
        return result;
}

