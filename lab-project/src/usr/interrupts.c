// Contains all interrupt handlers for ports (EXTI), timers, and ADC/DAC

#include <stdio.h>
#include "diag/Trace.h"
#include "cmsis/cmsis_device.h"
#include <stm32f051x8.h>

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

volatile double secondEdge = 0;      // Holds the value of TIM2 when second rising edge detected
volatile uint8_t timerTriggered = 0; // Indicates if the first edge has been detected

extern volatile unsigned int frequency;  // Measured frequency value (global variable, def. in main.c)
extern volatile unsigned int resistance; // Measured resistance value (global variable, def. in main.c)

// Handler for if no second edge is detected; TIM2 will overflow
void TIM2_IRQHandler()
{
	if ((TIM2->SR & TIM_SR_UIF) != 0) // Check if update interrupt flag is set
	{
		trace_printf("\n*** Overflow! ***\n");
		TIM2->SR &= ~TIM_SR_UIF; // Clear update interrupt flag 
		TIM2->CR1 |= TIM_CR1_CEN; // Restart stopped timer
	}
}

// Starts TIM2 on first edge, calculates frequency on second 
void EXTI2_3_IRQHandler()
{
	if ((EXTI->PR & EXTI_PR_PR2) != 0) // Check if EXTI2 interrupt pending flag is set
	{
		if (timerTriggered == 0) // If first edge of square wave, start timer
		{
			TIM2->CNT = 0;
			TIM2->CR1 |= TIM_CR1_CEN;
			timerTriggered = 1;
		}
		else // Second edge detected : stop timer and calculate frequency
		{
			TIM2 ->CR1 &= ~(TIM_CR1_CEN); 			  // Stop timer
			secondEdge = TIM2->CNT ; 	  			  // Read TIM2 count
			frequency = SystemCoreClock / secondEdge; // Calculate frequency, set global var frequency

			timerTriggered = 0;
		}

		EXTI->PR |= EXTI_PR_PR2; // Clear EXTI2 interrupt pending flag
	}
}

// TODO: User button interrupt handler - needs to switch between PA1 and PA2 as EXTI interrupt source
// Will need to configure EXTI, PADIR differently in setup.c

// When ADC is finished, moves output into DAC and starts conversion
//void ADC1_IRQHandler()
//{
// TODO
//}
//
// When DAC is finished, moves output into PA4 output (to 4n35)
//void ADC1_IRQHandler()
//{
// TODO
//}

#pragma GCC diagnostic pop
