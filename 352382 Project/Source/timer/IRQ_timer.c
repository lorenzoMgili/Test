/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "LPC17xx.h"
#include "timer.h"
#include "led.h"

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

void TIMER0_IRQHandler (void)
{
	static uint8_t position = 7;
	static uint8_t sw_count = 0;
	sw_count++;	
	if(sw_count == 2){
		
		LED_Off(position);
		if(position == 7)
			position = 2;
		else
			position++;
		LED_On(position);
			
		sw_count = 0;
	}
	/* alternatively to LED_On and LED_off try to use LED_Out */
	//LED_Out((1<<position)|(led_value & 0x3));							
	/* LED_Out is CRITICAL due to the shared led_value variable */
	/* LED_Out MUST NOT BE INTERRUPTED */
  LPC_TIM0->IR = 0x3F;		/* clear interrupt flag (clear all bits in IR) */
	/* an alternative instruction clearing only the bit set to 1 is LPC_TIM0->IR = LPC_TIM0->IR; */
  return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER1_IRQHandler (void)
{
  LPC_TIM1->IR = 0x3F;			/* clear interrupt flag */
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
