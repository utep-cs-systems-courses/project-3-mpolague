/** \file shapemotion.c
 *  \brief This is a simple shape motion demo.
 *  This demo creates two layers containing shapes.
 *  One layer contains a rectangle and the other a circle.
 *  While the CPU is running the green LED is on, and
 *  when the screen does not need to be redrawn the CPU
 *  is turned off along with the green LED.
 */  
#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include <buzzer.h>

#define GREEN_LED BIT6

char score1 = '0'; //player 1 score
char score2 = '0'; // player 2 score
short goal = 1;



AbRect middle = {abRectGetBounds, abRectCheck, {61, 0}};
AbRect paddle = {abRectGetBounds, abRectCheck, {15, 3}};

AbRectOutline fieldOutline = {	/* playing field */
  abRectOutlineGetBounds, abRectOutlineCheck,   
  {screenWidth/2 -2, screenHeight/2-2}
};

Layer fieldLayer = {		/* playing field as a layer */
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2},/**< center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_BLACK,
  0
};

//THIS IS THE BLACK LINE IN THE MIDDLE OF THE SCREEN
Layer layer3 = {
  (AbShape *)&middle,
    {(screenWidth/2), (screenHeight/2)}, //line is set horizontally accross the screen
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_BLACK, //line is black
  &fieldLayer
};


//THIS IS FOR THE BOTTOM LEFT PAD
Layer layer2 = {		/**< player 2 */
  (AbShape *)&paddle,
  {(screenWidth/4), (screenHeight-6)}, //set close to the bottom left of the screen
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_GREEN, //line is green
  &layer3,
};

//THIS IS FOR THE ORANGE PAD ON THE TOP MIDDLE 
Layer layer1 = {		/**< player 1 */
  (AbShape *)&paddle,
  {screenWidth/2, (6)}, 
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_ORANGE, //pad is orange
  &layer2,
};

//BALL STARTS AT THE CENTER OF THE SCREEN

/*
  layer0:
    

         jmp layer1;
 */
Layer layer0 = {	    
  (AbShape *)&circle14,
  {(screenWidth/2), (screenHeight/2)}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_BLACK, //ball is set to black
  &layer1,
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */

/*
  MovLayer_s:
             
 */
typedef struct MovLayer_s {
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

/* initial value of {0,0} will be overwritten */
//player 2
MovLayer ml2 = { &layer2, {5,10}, 0 }; /**< not all layers move */
//player 1
MovLayer ml1 = { &layer1, {5,5}, 0 }; 
//ball
MovLayer ml0 = { &layer0, {5,5}, 0 }; 

void movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);			/**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8);			/**< disable interrupts (GIE on) */


  for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], 
		bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
	Vec2 pixelPos = {col, row};
	u_int color = bgColor;
	Layer *probeLayer;
	for (probeLayer = layers; probeLayer; 
	     probeLayer = probeLayer->next) { /* probe all layers, in order */
	  if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
	    color = probeLayer->color;
	    break; 
	  } /* if probe check */
	} // for checking all layers at col, row
	lcd_writeColor(color); 
      } // for col
    } // for row
  } // for moving layer being updated
}	  

//This method was created as a helper to know when to exit the game.
/*
  .word default; 
  keepTrackScore:
                  cmp #4, &s; s-4 will borrow if s < 4
                  mov #s, r12
                  add r12, r12; 2*s
                  mov jt(r12), r0; jmp to jt[s] <- not sure if needed
  Case1:
     add #1, r12; s is in r12
     jmp esac;
  Case2:
     add #1, r12; 
     jmp esac;
  Case3:
     add #1, r12;
     jmp esac;
  Esac:
     pop r0     
 */
int keepTrackScore(){
  int s = 0;
  switch(s){
  case 1:
    s++;
    break;
  case 2:
    s++;
    break;
  case 3:
    s++;
    break;
    //this will terminate game
  }
  return 0;
}

Region fence = {{0,LONG_EDGE_PIXELS}, {SHORT_EDGE_PIXELS, LONG_EDGE_PIXELS}}; /**< Create a fence region */

/** Advances a moving shape within a fence
 *  
 *  \param ml0 The moving ball
 *  \param ml1 The moving paddle player 2
 *  \param ml2 The moving paddle player 1
 *  \param fence The region which will serve as a boundary for ml0
 */
void mlAdvance(MovLayer *ml, MovLayer *ml1, MovLayer *ml2, Region *fence)
{

  drawString5x7((screenWidth/2)/1.11, (screenHeight+140)/2, "Pong Game!", COLOR_WHITE, COLOR_BLACK);
    
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;


  //gets the row height
  int rowH = ml ->layer->posNext.axes[1];

  
  //gets the width of bottom 1
  int colH = ml->layer->posNext.axes[0];


  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++) {
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ) {
	    int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	    newPos.axes[axis] += (2*velocity); //had to put 2 because if I put something smaller, score would update awkwardly, if put something greater, it left pieces of the ball displayed 
      buzzer_set_period(1);
      }	/**< if outside of fence */

      //checks if ball hits player on bottom
      if((rowH >= 135) && (colH <= ml2->layer->posNext.axes[0] + 15 && colH >= ml2->layer->posNext.axes[0] - 15)){
        //int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
        ml->layer->color = COLOR_BLACK;
        int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
        ml->velocity.axes[0] += 1;
        newPos.axes[axis] += (2 * velocity);
        int redrawScreen = 1;
        
      }


      //CHECK IF BALL HITS PLAYER ON TOP
      else if((rowH <= 21) && (colH <= ml1->layer->posNext.axes[0] + 15 && colH >= ml1->layer->posNext.axes[0] - 15)){
        ml->layer->color = COLOR_GREEN;
        int velocity = ml->velocity.axes[axis] = ml->velocity.axes[axis];
        ml->velocity.axes[0] += 1;
        newPos.axes[axis] += (2 * velocity);
        int redrawScreen = 0; //no need to be redrawn
        
      }
      
      //we check if it hits the upper region
      else if((rowH == 20)){
        ml2->layer->color = COLOR_RED;
        score2++; // change char 0 to char 1;
	drawString5x7((screenWidth/2)/.9, (screenHeight-40/2), "score!", COLOR_WHITE, COLOR_BLACK);
	
	drawChar5x7((screenWidth/2)/2, (screenHeight/2)/+15, score2, COLOR_YELLOW, COLOR_BLACK);

        goal = 1;
	      newPos.axes[0] = screenWidth/2;
	      newPos.axes[1] = (screenHeight/2);
	      ml->velocity.axes[0] = 5;
	      ml->layer->posNext = newPos;
	      int redrawScreen = 1;
        
      }

      int redrawScreen = 1;

      if(goal != 1){
        ml->layer->posNext = newPos;
      }
      
    } /**< for axis */
    ml->layer->posNext = newPos;
  } /**< for ml */
}


u_int bgColor = COLOR_WHITE; //color of background is set to white
//set to true if screen needs to be redrawn 
int redrawScreen = 1;

Region fieldFence;		/**< fence around playing field  */


/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{

  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(1);
 

  shapeInit();

  layerInit(&layer0);
  layerDraw(&layer0);
  drawTriangle((screenWidth/2)-60, screenHeight/2, 20, COLOR_ORANGE);
  
  layerGetBounds(&fieldLayer, &fieldFence);


  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */

  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  P1OUT |= GREEN_LED;

  //THIS ENABLES THE LIGHT TO TURN OFF -> CANT MAKE IT WORK ON ITS OWN
  /*for(;;) { 
    P1OUT = (1 & p2sw_read());
    }*/

  
  for(;;) { 
    while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
      or_sr(0x10);	      /**< CPU OFF */
    }
    P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
    redrawScreen = 0;
    movLayerDraw(&ml0, &layer0);
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  if (count == 15) {
    mlAdvance(&ml0, &ml1, &ml2, &fieldFence);
    if (p2sw_read())
      redrawScreen = 1;
    count = 0;
  } 
  P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}
