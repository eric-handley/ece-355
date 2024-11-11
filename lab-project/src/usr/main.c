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

volatile unsigned int frequency = 0;  // Measured frequency value (global variable)
volatile unsigned int resistance = 0; // Measured resistance value (global variable)

void poll_ADC(void);
void start_DAC(void);

extern void SystemClock48MHz();
extern void GPIOA_Init();
extern void TIM2_Init();
extern void TIM3_Init();
extern void TIM3_Reset();
extern void EXTI_Init();
extern void oled_Init();
extern void refresh_OLED();

int main(int argc, char* argv[])
{
	SystemClock48MHz();

	GPIOA_Init();	// Initialize I/O port PA, along with ADC and DAC
	TIM2_Init();	// Initialize timer TIM2 - signal edge counter
	TIM3_Init();	// Initialize timer TIM3 - display refresh rate counter
	EXTI_Init();	// Initialize EXTI

	oled_Init();    // Initialize OLED setup, including I/O port PB

	while (1)
	{
		TIM3_Reset(); // Sets TIM3 for ~100 ms to get ~10 frames/sec refresh rate
		
		poll_ADC();
		start_DAC();
		
		refresh_OLED(frequency, resistance); // Refresh OLED with frequency and resistance values

		while(TIM3->CR1 & TIM_CR1_CEN); // While TIM3 not zero (CEN not reset)
	}

	return 0;
}

// Polls ADC (input PA5) and calculates resistance value if conversion is done
void poll_ADC() 
{
	if(ADC1->ISR & ADC_ISR_ADRDY_Msk) {
		ADC1->CR |= ADC_CR_ADSTART;
	}

	if(ADC1->ISR & ADC_ISR_EOC_Msk) {
		resistance = ((float)(ADC1->DR) / 4095) * 5000;
	}
}

void start_DAC() {
	// TODO
}

#pragma GCC diagnostic pop
