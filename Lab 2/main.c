/************** ECE2049 DEMO CODE ******************/
/**************  13 March 2019   ******************/
/***************************************************/

//define libraries
#include <msp430.h>
#include <stdio.h>
#include "peripherals.h"

//define global counter variables used for time keeping
long unsigned int timer_cnt=0;//this is the timer that inreses every .001 seconds
long unsigned int prev_time=0;//used to keep track of time intervals
char tdir = 1;//timer count direction

//
int SongNote = 0;//index for the song we play
uint8_t led; // char used to turn on and off leds

void runtimerA2(void);// start timer which will cause an interrupt to run every .001 seconds and increment timer_cnt
void stoptimerA2(int reset); //stop timer
__interrupt void TimerA2_ISR (void); //interrupt service routine which will run every .001 seconds and increment timer_cnt

void configUserLED(char inbits);//sets up leds
void configUserButtons(void);//sets up user buttons
uint8_t getState(void); //uint8_t


//states for state machine
typedef enum {
    WELCOME = 0,
    COUNTDOWN = 1,
    PLAY_NOTE = 2,
    LIFE_LOST = 3,
    GAME_OVER = 4,
    YOU_WIN = 5,
    EXIT = 6
} GAME_STATE;

//struct used to store the song played
struct song {
    int size;
    unsigned int beats[100]; // number of eighth notes per note
    unsigned int frequency[100]; // Frequency of each note
    };


unsigned char currKey;// stores launchpad button input in char

int main(void) {
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
    _BIS_SR(GIE); //enables interrupts
    runtimerA2(); //start timer

    //configure I/O before starting
    initLeds();
    configDisplay();
    configKeypad();
    configUserButtons();


    int speed = 80; //speed variable for game- decrease to make the game harder
    int lives = 5; //number of lives or mistakes player has

    unsigned char currKey = getKey();// poll number pad for input and store it

    uint8_t currButton = 0; //launchpad button

    // the song instancec we use to play the music
    struct song miiSong = {
      .size = 100,
      .beats = {1  ,1, 1,  1, 1,  1, 1,  1,  1,  1, 4, 1,  1,  1,  1,  1, 1, 1, 1, 1,  3,  1,  1, 1,2, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1,  1,  1, 1,2, 1,  1, 1,  1,2, 2,  2,  1, 1, 1,  1, 1,  1,1, 1, 1,   1,  1, 1, 1,  1,  1, 1,1, 1,  1,  1, 1, 1, 1, 1,  4, 1,  2,1, 1,  1,  1, 2,  1,  1,   1, 1,   1, 1,   2,  1,  1,  1,  1,  1,  1, 1},
      .frequency = {370,0,440,554,0,440,370,294,294,294,0,208,294,370,440,550,0,440,0,370,659,622,587,0,0,415,0,554,370,0,554,0,415,0,554,0,392,370,0,330,0,262,262,262,0,0,277,277,261,0,0,311,294,370,0,440,554,0,440,0,370,294,294,294,0,659,659,659,0,0,370,440,554,0,440,0,370,659,587,0,0,493,392,293,262,493,415,293,440,370,263,247,349,293,247,230,230,230}
    };

    //set start state
    GAME_STATE my_state = WELCOME;

    while(1){
        //get keypad inputs
        currKey = getKey();

        switch(my_state){
            case WELCOME: //display Welcome Screen
               setLeds(0);
               Graphics_drawStringCentered(&g_sContext, "Guitar Hero", AUTO_STRING_LENGTH, 48, 10, TRANSPARENT_TEXT);
                   Graphics_drawStringCentered(&g_sContext, "'&`", AUTO_STRING_LENGTH, 48, 20, TRANSPARENT_TEXT);
                  Graphics_drawStringCentered(&g_sContext, "#", AUTO_STRING_LENGTH, 48, 30, TRANSPARENT_TEXT);
                 Graphics_drawStringCentered(&g_sContext, "#", AUTO_STRING_LENGTH, 48, 40, TRANSPARENT_TEXT);
                 Graphics_drawStringCentered(&g_sContext, "_#_", AUTO_STRING_LENGTH, 48, 50, TRANSPARENT_TEXT);
                 Graphics_drawStringCentered(&g_sContext, "( # )", AUTO_STRING_LENGTH, 48, 60, TRANSPARENT_TEXT);
                 Graphics_drawStringCentered(&g_sContext, "/ 0 \\", AUTO_STRING_LENGTH, 48, 70, TRANSPARENT_TEXT);
               Graphics_drawStringCentered(&g_sContext, "( === )", AUTO_STRING_LENGTH, 48, 80, TRANSPARENT_TEXT);
               Graphics_drawStringCentered(&g_sContext, "`---'", AUTO_STRING_LENGTH, 48, 90, TRANSPARENT_TEXT);
               Graphics_flushBuffer(&g_sContext);

               //if player presses start button begin the game
               if (currKey == '*')
               {
                   my_state = COUNTDOWN;
                   Graphics_clearDisplay(&g_sContext); // Clear the display
               }
               else
               {
                   my_state = WELCOME;
               }
               //keep track of time
               prev_time = timer_cnt;
               break;
            case COUNTDOWN: //counts down to start of song
                Graphics_flushBuffer(&g_sContext); //clear dispaly
                //use timer A2 see how much time has passed and count down and display it
                 if ((timer_cnt - prev_time) > (500)){
                     setLeds(0);
                     Graphics_clearDisplay(&g_sContext); // Clear the display
                     prev_time = timer_cnt;
                     currButton = 0;
                     my_state = PLAY_NOTE;
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
                 //once countdown is over, move on to play note state
                 if ((my_state != EXIT)&&(my_state != PLAY_NOTE)){
                     my_state = COUNTDOWN;
                 }
                 break;
            case PLAY_NOTE: //play the note and listen for player input
                //if song is over, player has won and move to win state
                if(SongNote == (miiSong.size)){
                    my_state = YOU_WIN;
                }

                //play note for its duration while lighting corresponding LED and listening for button
                if((timer_cnt - prev_time) <= miiSong.beats[SongNote]*speed){ //40
                    //check for button pressed and or result to keep track of past buttons pressed
                    currButton |= getState();
                    Graphics_clearDisplay(&g_sContext);

                    //calculate which led to light and button to listen for
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
                    else if (miiSong.frequency[SongNote] == 0)
                    {
                        led = 0;
                    }
                    setLeds(led);
                    //check for button pressed

                }
                //note is finished playing
                else{
                    //reset time interval
                    prev_time = timer_cnt;
                    //turn off buzzer
                    BuzzerOff();
                    //increment song index
                    SongNote++;

                    //if player misses note
                    if ((led != currButton)&&(miiSong.frequency[SongNote] != 0))
                    {
                        //move to life lost state
                        my_state = LIFE_LOST;
                    }
                    //reset button
                    currButton = 0;
                }

            break;
            // flash LEDs and buzz buzzer to show player that they messed up
            case  LIFE_LOST:
                //play dissonant note
               BuzzerOn(116);

               //flash all LEDs
               if ((timer_cnt-prev_time) < 33){
                   setLeds(15);
               }
               else if((timer_cnt-prev_time) < 66){
                   setLeds(0);
               }
               else if ((timer_cnt-prev_time) < 100){
                   setLeds(15);
               }
               else if ((timer_cnt-prev_time) < 133){
                   setLeds(0);
               }
               once flashing is over
               else{
                   lives--;//eliminate life
                   prev_time = timer_cnt; //reset timer
                   my_state = PLAY_NOTE; //go back to playing note
               }
               //if player is out of lives, end game
               if (lives <= 0) {
                   my_state = GAME_OVER;
                   prev_time = timer_cnt;
               }

                break;
                //game over screen humiliating player
            case GAME_OVER:
                //turn off LEDs
                setLeds(0);
                //humiliate player
                if ((timer_cnt - prev_time) < (400)){
                    Graphics_drawStringCentered(&g_sContext, ":( you lose :(", AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "xD xD your bad xD xD", AUTO_STRING_LENGTH, 48, 25, OPAQUE_TEXT);
                }
                //once humiliation is over, exit
                else{
                    my_state = EXIT;
                    prev_time = 0;
                }
                //clear LCD
                Graphics_flushBuffer(&g_sContext);
                break;
                //if player wins, congratulate them
            case YOU_WIN:
                setLeds(0);
                //congratulate player for 4 seconds
                if ((timer_cnt - prev_time) < (400)){
                    Graphics_drawStringCentered(&g_sContext, ":) YOU WIN :)", AUTO_STRING_LENGTH, 48, 15, OPAQUE_TEXT);
                    Graphics_drawStringCentered(&g_sContext, "^-^ Congrats! ^-^", AUTO_STRING_LENGTH, 48, 25, OPAQUE_TEXT);

                }
                //once done, exit
                else{

                    my_state = EXIT;
                    prev_time = 0;
                }

                //clear LCD
                Graphics_flushBuffer(&g_sContext);
                break;
                //exits game and rests all vareables
            case EXIT:

                setLeds(0); //turn off leds
                BuzzerOff(); //turn off buzzer
                Graphics_clearDisplay(&g_sContext); // Clear the display
                currButton = 0; //clear button
                SongNote = 0; //clear note
                lives = 5; //reset lives
                my_state = WELCOME; //go back to welcome screen
                break;
        }

        //poll for abort button every loop
        currKey = getKey();
        if (currKey == '#') //#
        {
            my_state = EXIT;
        }
    }
}

// start timer which will cause an interrupt to run every .001 seconds and increment timer_cnt
void runtimerA2(void){
// Use ACLK, 16 Bit, up mode, 1 divider
    TA2CTL = TASSEL_1 + MC_1 + ID_0;
    TA2CCR0 = 327; // 327+1 = 328 ACLK tics = ~1/100 seconds
    TA2CCTL0 = CCIE; // TA2CCR0 interrupt enabled
}

//stop timer
void stoptimerA2(int reset)
{
    TA2CTL = MC_0; // stop timer
    TA2CCTL0 &= ~CCIE; // TA2CCR0 interrupt disabled
        if(reset)
            timer_cnt=0;
}
//interrupt service routine which will run every .001 seconds and increment timer_cnt#pragma vector=TIMER2_A0_VECTOR
__interrupt void TimerA2_ISR (void){
    if(tdir){
        timer_cnt++;
    }
    else
        timer_cnt--;
}

//set up user LEDs
void configUserLED(char inbits){
    P1SEL &= (BIT0);
    P4SEL &= (BIT7);

    P1DIR |= (BIT0);
    P4DIR |= (BIT7);

    P1OUT = inbits &= BIT0;
    P2OUT = inbits &= BIT1;

}

//set up 4 buttons on launch pad
void configUserButtons(void){
    P7SEL &= ~(BIT0|BIT4); //S1, S4
    P3SEL &= ~(BIT6); //S2
    P2SEL &= ~(BIT2); //S3

    P7DIR &= ~(BIT0|BIT4); //S1, S4
    P3DIR &= ~(BIT6); //S2
    P2DIR &= ~(BIT2); //S3

    P7REN |= (BIT0|BIT4); //S1, S4
    P3REN |= (BIT6); //S2
    P2REN |= (BIT2); //S3

    P7OUT |= (BIT0|BIT4); //S1, S4
    P3OUT |= (BIT6); //S2
    P2OUT |= (BIT2); //S3
}

//get which buttons have been pressed and put it in a char
uint8_t getState(void){
        uint8_t result = 0x00;
        if (~P7IN & BIT0) {//sw1
            result |= BIT3;
        }
        if (~P3IN & BIT6) {
            result |= BIT2;
        }
        if (~P2IN & BIT2) {
            result |= BIT1;
        }
        if (~P7IN & BIT4) { //bit 4 is set
            result |= BIT0;
        }

        return result;
}
