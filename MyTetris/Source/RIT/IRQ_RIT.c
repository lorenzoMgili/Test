/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "LPC17xx.h"
#include "RIT.h"
//#include "led.h"
#include "timer.h"
#include "music.h"
#include "adc.h"

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
// beat 1/4 = 1.65/4 seconds
#define RIT_SEMIMINIMA 8
#define RIT_MINIMA 16
#define RIT_INTERA 32


void RIT_IRQHandler (void)
{
	static int currentNote = 0;
	static int effectNote = 0;
	static int ticks = 0;
	

	
	if(!isNotePlaying())
	{
		++ticks;
		if(ticks == UPTICKS)
		{
			ticks = 0;
			if (playingEffect.length != 0) {
				playNote(playingEffect.notes[effectNote++]);
				if (effectNote >= playingEffect.length){
					playEffect(NO_EFFECT);
					effectNote = 0;
				}
			} else {
				playNote(TETRIS_MUSIC.notes[currentNote++]);
			}
			
		}
	}
	
	if(currentNote == TETRIS_MUSIC.length)
	{

		//disable_RIT();		// stop the music
		currentNote = 0;		// restart the music
	}
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
}
/******************************************************************************
**                            End Of File
******************************************************************************/
