#include "button.h"
#include "LPC17xx.h"

#include "game.h"
//extern int down;

void EINT0_IRQHandler (void)	  	/* INT0														 */
{		
	
	LPC_SC->EXTINT &= (1 << 0);     /* clear pending interrupt         */
}


void EINT1_IRQHandler (void)	  	/* KEY1														 */
{
	togglePause();
	LPC_SC->EXTINT &= (1 << 1);     /* clear pending interrupt         */
}

void EINT2_IRQHandler (void)	  	/* KEY2														 */
{
	setHardDrop();
  LPC_SC->EXTINT &= (1 << 2);     /* clear pending interrupt         */    
}


