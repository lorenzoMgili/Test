#include "button.h"
#include "LPC17xx.h"

#include "game.h"


void EINT0_IRQHandler (void)	  	/* INT0														 */
{		
	
	LPC_SC->EXTINT &= (1 << 0);     /* clear pending interrupt         */
}


void EINT1_IRQHandler (void)	  	/* KEY1														 */
{
	if (!isButton1Paused()){
		togglePause();
		pauseButton1();
	}
	LPC_SC->EXTINT &= (1 << 1);     /* clear pending interrupt         */
}

void EINT2_IRQHandler (void)	  	/* KEY2														 */
{
	if (!isButton2Paused()){
		setHardDrop();
		pauseButton2();
	}
  LPC_SC->EXTINT &= (1 << 2);     /* clear pending interrupt         */    
}


