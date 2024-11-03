//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------
// School: University of Victoria, Canada.
// Course: ECE 355 "Microprocessor-Based Systems".
//
// See "system/include/cmsis/stm32f051x8.h" for register/bit definitions.
// See "system/src/cmsis/vectors_stm32f051x8.c" for handler declarations.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include "diag/Trace.h"
#include "cmsis/cmsis_device.h"
#include <stm32f051x8.h>

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

volatile double secondEdge = 0; // Holds the value of TIM2 when second rising edge detected
volatile uint8_t timerTriggered = 0; // Indicates if the first edge has been detected

volatile uint32_t frequency = 0;  // Measured frequency value (global variable)
volatile uint32_t resistance = 0; // Measured resistance value (global variable)

/*****************************************************************/

int main(int argc, char* argv[])
{
	SystemClock48MHz();

	GPIOA_Init();	// Initialize I/O port PA
	TIM2_Init();	// Initialize timer TIM2
	EXTI_Init();	// Initialize EXTI

	oled_config(); 

	while (1)
	{
		refresh_OLED(frequency, resistance); // Refresh OLED with frequency and resistance values
		for(int i = 0; i < 10000; i++); 	 // Arbitrary delay before next refresh
	}

	return 0;
}

/* This handler is declared in system/src/cmsis/vectors_stm32f051x8.c */
void TIM2_IRQHandler()
{
	/* Check if update interrupt flag is indeed set */
	if ((TIM2->SR & TIM_SR_UIF) != 0)
	{
		trace_printf("\n*** Overflow! ***\n");

		/* Clear update interrupt flag */
		// Relevant register: TIM2->SR
		TIM2->SR &= ~TIM_SR_UIF;

		/* Restart stopped timer */
		// Relevant register: TIM2->CR1
		TIM2->CR1 |= TIM_CR1_CEN;
	}
}

/* This handler is declared in system/src/cmsis/vectors_stm32f051x8.c */
void EXTI2_3_IRQHandler()
{
	/* Check if EXTI2 interrupt pending flag is indeed set */
	if ((EXTI->PR & EXTI_PR_PR2) != 0)
	{
		// Iff first edge of square wave, start timer
		if (timerTriggered == 0)
		{
			TIM2->CNT = 0;
			TIM2->CR1 |= TIM_CR1_CEN;
			timerTriggered = 1;
		}
		else
		{
			// Second edge detected : stop timer and calculate frequency
			TIM2 ->CR1 &= ~(TIM_CR1_CEN); // Stop timer
			secondEdge = TIM2->CNT ; // Read TIM2 count

			// Calculate period and frequency
			double period = secondEdge; // In clock cycles
			double frequency = SystemCoreClock / secondEdge;

			trace_printf("Period is %.2f cycles\n", period);
			trace_printf("Frequency is %.2f Hz \n", frequency);

			timerTriggered = 0;
		}

		// Clear EXTI2 interrupt pending flag
		EXTI->PR |= EXTI_PR_PR2;
	}
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
