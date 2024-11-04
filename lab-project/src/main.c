// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.

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

int main(int argc, char* argv[])
{
	SystemClock48MHz();

	GPIOA_Init();	// Initialize I/O port PA
	TIM2_Init();	// Initialize timer TIM2 - signal edge counter
	TIM3_Init();	// Initialize timer TIM3 - display refresh rate counter
	EXTI_Init();	// Initialize EXTI

	oled_Init();    // Initialize OLED setup, including I/O port PB

	while (1)
	{
		TIM3_Reset(); // Sets TIM3 for ~100 ms to get ~10 frames/sec refresh rate 
		refresh_OLED(frequency, resistance); // Refresh OLED with frequency and resistance values

		while(~(TIM3->SR & TIM_SR_UIF_Msk)); // While TIM3 not zero (UIF not set)
	}

	return 0;
}

/* This handler is declared in system/src/cmsis/vectors_stm32f051x8.c */
void TIM2_IRQHandler()
{
	if ((TIM2->SR & TIM_SR_UIF) != 0) // Check if update interrupt flag is indeed set
	{
		trace_printf("\n*** Overflow! ***\n");

		TIM2->SR &= ~TIM_SR_UIF; // Clear update interrupt flag 
		TIM2->CR1 |= TIM_CR1_CEN; // Restart stopped timer
	}
}

/* This handler is declared in system/src/cmsis/vectors_stm32f051x8.c */
void EXTI2_3_IRQHandler()
{
	if ((EXTI->PR & EXTI_PR_PR2) != 0) // Check if EXTI2 interrupt pending flag is indeed set
	{
		if (timerTriggered == 0) // Iff first edge of square wave, start timer
		{
			TIM2->CNT = 0;
			TIM2->CR1 |= TIM_CR1_CEN;
			timerTriggered = 1;
		}
		else // Second edge detected : stop timer and calculate frequency
		{
			TIM2 ->CR1 &= ~(TIM_CR1_CEN); // Stop timer
			secondEdge = TIM2->CNT ; // Read TIM2 count

			frequency = SystemCoreClock / secondEdge; // Calculate frequency, set global var frequency

			timerTriggered = 0;
		}

		EXTI->PR |= EXTI_PR_PR2; // Clear EXTI2 interrupt pending flag
	}
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
