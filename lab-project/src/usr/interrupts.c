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

void calculate_Frequency()
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
}

// Handler for if no second edge is detected; TIM2 will overflow
void TIM2_IRQHandler()
{
	if ((TIM2->SR & TIM_SR_UIF) != 0) 			// Check if update interrupt flag is set
	{
		trace_printf("\n*** Overflow! ***\n");
		TIM2->SR &= ~TIM_SR_UIF; 				// Clear update interrupt flag 
		TIM2->CR1 |= TIM_CR1_CEN; 				// Restart stopped timer
	}
}

void EXTI0_1_IRQHandler()
{
	if ((EXTI->PR & EXTI_PR_PR0_Msk) != 0) // User button interrupt (falling edge)
	{
		EXTI->IMR ^= EXTI_IMR_MR1 | EXTI_IMR_MR2; 	// Toggle each of EXTI1, EXTI2 interrupt mask
		EXTI->PR |= EXTI_PR_PR0; 					// Clear EXTI0 interrupt pending flag
	}

	if ((EXTI->PR & EXTI_PR_PR1_Msk) != 0) // Timer circuit interrupt
	{
		calculate_Frequency();
		EXTI->PR |= EXTI_PR_PR1; // Clear EXTI1 interrupt pending flag
	}
}

void EXTI2_3_IRQHandler()
{
	if ((EXTI->PR & EXTI_PR_PR2_Msk) != 0) // Check if EXTI2 interrupt pending flag is set
	{
		calculate_Frequency();
		EXTI->PR |= EXTI_PR_PR2; // Clear EXTI2 interrupt pending flag
	}
}

#pragma GCC diagnostic pop
