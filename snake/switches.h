#ifndef switches_included
#define switches_included

#define SW1 BIT0 /* switch1 */
#define SW2 BIT1 /* switch2 */
#define SW3 BIT2 /* switch3 */
#define SW4 BIT3 /* switch4 */

#define SWITCHES (SW1 | SW2 | SW3 | SW4) /* 4 switch on this board */

void switch_init();
void switch_interrupt_handler();

extern char switch1_state, switch2_state, switch3_state, switch4_state;
extern char switch_state_changed; /* effectively boolean */

#endif // included
