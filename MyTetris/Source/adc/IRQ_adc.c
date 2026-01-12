/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_adc.c
** Last modified Date:  20184-12-30
** Last Version:        V1.00
** Descriptions:        functions to manage A/D interrupts
** Correlated files:    adc.h
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/

#include "LPC17xx.h"
#include "adc.h"
#include "game.h"
#include "timer.h"

/*----------------------------------------------------------------------------
  A/D IRQ: Executed when A/D Conversion is ready (signal from ADC peripheral)
 *----------------------------------------------------------------------------*/

static volatile float current=0.0; 
static volatile float last=1.0;

void ADC_IRQHandler(void) {
  volatile uint32_t ad_stat = LPC_ADC->ADGDR;
  volatile uint16_t result = (ad_stat >> 4) & 0xFFF;
	current=((float) result * 4.0f)/0xFFF+1.0f;
  if(current != last){
		last = current;
		updateSpeed(current);
  }	
	
}
