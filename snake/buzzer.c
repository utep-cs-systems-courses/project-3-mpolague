#include <msp430.h>
#include "libTimer.h"
#include "buzzer.h"

void buzzer_init()
{
    /* 
       Direct timer A output "TA0.1" to P2.6.  
        According to table 21 from data sheet:
          P2SEL2.6, P2SEL2.7, anmd P2SEL.7 must be zero
          P2SEL.6 must be 1
        Also: P2.6 direction must be output
    */
    timerAUpmode();		/* used to drive speaker */
    P2SEL2 &= ~(BIT6 | BIT7);
    P2SEL &= ~BIT7; 
    P2SEL |= BIT6;
    P2DIR = BIT6;		/* enable output to speaker (P2.6) */
}

void buzzer_set_period(short cycles) /* buzzer clock = 2MHz.  (period of 1k results in 2kHz tone) */
{
  CCR0 = cycles; 
  CCR1 = cycles >> 1;		/* one half cycle */
}

const unsigned short C3 = 15289.35;
const unsigned short Dd3 = 14431.06;
const unsigned short D3 = 13621.19;
const unsigned short Eb3 = 15289.35;
const unsigned short E3 = 15289.35;
const unsigned short F3 = 15289.35;
const unsigned short Gb3 = 15289.35;
const unsigned short G3 = 15289.35;
const unsigned short Ab3 = 15289.35;
const unsigned short A3 = 15289.35;

//array of notes (songs)
const int song[9] = {C3, Dd3, D3, Eb3, E3, F3, Gb3, G3, Ab3};

void play_song(){
  for(int i = 0; i < 8; i++){
    for(int j = 0; j < 30000; j++){
      buzzer_set_period(song[i]);
    }
    buzzer_set_period(0); //turns off buzzer
    return;
  }
}


const int song2[14] = {200,200,200,200,200,200,200,200,200,200,200,200,200,200,200};

void play_song2(){
  for(int i = 0; i < 13; i++){
    for(int j = 0; j < 30000; j++){
      buzzer_set_period(song2[i]);
    }
    buzzer_set_period(0); //turns off buzzer
    return;
  }
}
    
    
  

