#include "button.h"
#include "LPC17xx.h"


#include "../led/led.h"
#include "../timer/timer.h"
#include "RIT.h"
#include "adc.h"


extern int Look_and_say(int val);
void EINT0_IRQHandler (void)	  	/* INT0														 */
{
	disable_RIT();
	int result = getValue();
	result= Look_and_say(result);
	displayNumber(result);
	
	
	LPC_SC->EXTINT &= (1 << 0);     /* clear pending interrupt         */
}


void EINT1_IRQHandler (void)	  	/* KEY1														 */
{
  
	LPC_SC->EXTINT &= (1 << 1);     /* clear pending interrupt         */
}

void EINT2_IRQHandler (void)	  	/* KEY2														 */
{
	
  LPC_SC->EXTINT &= (1 << 2);     /* clear pending interrupt         */  
	
}


