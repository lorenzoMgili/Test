/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               main.c
** Descriptions:            The GLCD application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-11-7
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             Paolo Bernardi
** Modified date:           03/01/2020
** Version:                 v2.0
** Descriptions:            basic program for LCD and Touch Panel teaching
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "GLCD.h" 
#include "timer.h"
#include "game.h"


#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif

void joystick_init(void) {
	/* joystick up functionality */
    LPC_PINCON->PINSEL3 &= ~(3<<26);	//PIN mode GPIO (00b value per P1.29)
	LPC_GPIO1->FIODIR   &= ~(1<<29);	//P1.25 Input (joysticks on PORT1 defined as Input) 
}



int main(void)	{

	SystemInit();  												/* System Initialization  			*/
  BUTTON_init();                       	/* Initialize Button interrupts	*/
	
  LCD_Initialization();									/* LCD Initialization  					*/
  joystick_init();
	
	initGame();														/* Game Initialization  				*/
	init_timer(0, TIMERFREQ); 						/* Timer Initialization  				*/ 
	
	enable_timer(0);											/* Timer Enabling  							*/
	
	LPC_SC->PCON |= 0x1;									/* power-down	mode							*/
	LPC_SC->PCON &= ~(0x2);						
	SCB->SCR |= 0x2;											/* set SLEEPONEXIT */
	
	__ASM("wfi");

}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
