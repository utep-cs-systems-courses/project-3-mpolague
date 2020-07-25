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

#define GREEN_LED BIT6

char score1 = '0'; //player 1 score
char score2 = '0'; // player two score
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

Layer layer3 = {
  (AbShape *)&middle,
  {(screenWidth/2), (screenHeight/2)}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_RED,
  &fieldLayer
};
  

Layer layer2 = {		/**< player 2 */
  (AbShape *)&paddle,
  {(screenWidth/2), (screenHeight-6)}, //setting to bottom center of screen with 3 pixels from edge
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_RED,
  &layer3,
};

Layer layer1 = {		/**< player 1 */
  (AbShape *)&paddle,
  {screenWidth/2, (6)}, //3 pixels from top of screen
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_WHITE,
  &layer2,
};

Layer layer0 = {		/**< ball */
  (AbShape *)&circle14,
  {(screenWidth/2), (screenHeight/2)}, /**< bit below & right of center */
  {0,0}, {0,0},				    /* last & next pos */
  COLOR_ORANGE,
  &layer1,
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
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

  drawString5x7((screenWidth/2)/2, (screenHeight/2)/2, "Player2", COLOR_WHITE, COLOR_BLACK);
  drawString5x7((screenWidth/2)/2, (screenHeight-75), "Player1", COLOR_WHITE, COLOR_BLACK);
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;


  //gets the row hieght
  int rowH = ml ->layer->posNext.axes[1];
  //gets the width of player 1
  int colH = ml->layer->posNext.axes[0];


  for (; ml; ml = ml->next) {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++) {
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
	  (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ) {
	    int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	    newPos.axes[axis] += (2*velocity);
      buzzer_set_period(0);
      }	/**< if outside of fence */

      //checks if ball hits or player one
      if((rowH >= 135) && (colH <= ml2->layer->posNext.axes[0] + 15 && colH >= ml2->layer->posNext.axes[0] - 15)){
        //int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
        ml->layer->color = COLOR_BLACK;
        int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
        ml->velocity.axes[0] += 1;
        newPos.axes[axis] += (2 * velocity);
        int redrawScreen = 1;
        
      }//check if ball hits player 2
      else if((rowH <= 21) && (colH <= ml1->layer->posNext.axes[0] + 15 && colH >= ml1->layer->posNext.axes[0] - 15)){
        //int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
        ml->layer->color = COLOR_RED;
        int velocity = ml->velocity.axes[axis] = ml->velocity.axes[axis];
        ml->velocity.axes[0] += 1;
        newPos.axes[axis] += (2 * velocity);
        int redrawScreen = 1;
        
      }
      //we check if it hits the upper region
      else if((rowH == 20)){
        //int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
        ml2->layer->color = COLOR_RED;
	      //drawChar5x7(52,152, player1Score, COLOR_YELLOW, COLOR_BLACK);
	      newPos.axes[0] = screenWidth/2;
	      newPos.axes[1] = (screenHeight/2);
	      ml->velocity.axes[0] = 5;
	      ml->layer->posNext = newPos;
	      int redrawScreen = 1;
        
      }
      //check if ball hits lower region
      else if((rowH == 136)){
        //int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
        ml1->layer->color = COLOR_RED;
	      //drawChar5x7(120,152, player2Score, COLOR_GREEN, COLOR_BLACK);	   
	      newPos.axes[0] = screenWidth/2;
	      newPos.axes[1] = (screenHeight/2);
	      ml->velocity.axes[0] = 5;
	      ml->layer->posNext = newPos;
	      int redrawScreen = 1;
        
      }
      
    } /**< for axis */
    ml->layer->posNext = newPos;
  } /**< for ml */
}


u_int bgColor = COLOR_BLUE;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */


/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void main()
{
   
  P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(1);

  shapeInit();

  layerInit(&layer0);
  layerDraw(&layer0);


  layerGetBounds(&fieldLayer, &fieldFence);


  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */


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
