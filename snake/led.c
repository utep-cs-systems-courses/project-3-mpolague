#include <msp430.h>
#include "led.h"
#include "stateMachines.h"

unsigned char red_on = 0, green_on = 0;
unsigned char led_changed = 0;

static char redVal[] = {0, LED_RED}, greenVal[] = {0, LED_GREEN};

void led_init()
{
  P2DIR |= LEDS;// bits attached to leds are output
  //led_changed = 1;
  //led_update();
}

void led_update()
{
  if (led_changed) {
    char ledFlags = redVal[red_on] | greenVal[green_on];
    P2OUT &= (0xff^LEDS) | ledFlags; // clear bit for off leds
    P2OUT |= ledFlags;     // set bit for on leds
    led_changed = 0;
  }
}

//turns lights off
void lights_off()
{
  green_on = 0;
  red_on = 0;
  led_update();
}


int led_state(int redL, int greenL){
  char ledF = 0;

  ledF |= redL ? LED_RED : 0;

  
  ledF |= greenL ? LED_GREEN : 0;

  P2OUT &= (0Xff - LEDS) | ledF; //clears the bits

  
  P1OUT |= ledF; //set bits 
}
