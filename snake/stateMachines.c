#include <msp430.h>
#include <lcdutils.h>
#include "stateMachines.h"
#include "led.h"
#include "switches.h"
#include "buzzer.h"
#include "shape.h"
#include "libTimer.h"
#include "lcddraw.h"

int song3[] = {440}; //will play this note
int song4[] = {247}; //will play this second note

int i = 0; //will help move through the list
int j = 0; //will help move through the list 

char toggle_red(){
  static char state = 0;

  switch (state){
  case 0:
    red_on = 1; //will turn on
    state = 1; //will go to state 1
    break;
  case 1:
    red_on = 0; //will turn off
    state = 0; //will go back to state 0
    break;
  }
  return 1;
}

char toggle_green(){
  char changed = 0; //will help determine if light was changed

  if (red_on){
    green_on ^= 1;
    changed = 1; //will change the variable to 1 to show it changed
  }
  return changed; //will return 1 if the light was changed
}

void _state_advance(){
  char changed = 0;

  static enum {R=0, G=1} color = G;
  switch (color){
  case R: changed = toggle_red(); color = G; break;
  case G: changed = toggle_green(); color = R; break;
  }

  led_changed = changed;
  led_update();
}

void state_advance(){
  switch(switch_state){
  case 1:
    buzzer_set_period(0);
    buzzer_set_period(song3[i]); //will play first song
    switch_state = 0; //will go back to state 0
    break;
  case 2:
    //_state_advance();
    led_state(0,1); //wanted to turn on green led
    led_state(0,0); //wanted to turn off green led
    drawTriangle((screenWidth/2)-70, screenHeight/2, 20, COLOR_ORANGE);
    break;
  case 3:
    buzzer_set_period(0); //will turn off the sound completely   
    break;
    
  case 4:
    buzzer_set_period(song4[j]); //will play second song and move
    switch_state = 0; //will go back to state 0
    break;
  }
  switch_state = 0;
}
