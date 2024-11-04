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

void poll_Potentiometer(void);
void start_ADC(void);

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

	GPIOA_Init();	// Initialize I/O port PA
	TIM2_Init();	// Initialize timer TIM2 - signal edge counter
	TIM3_Init();	// Initialize timer TIM3 - display refresh rate counter
	EXTI_Init();	// Initialize EXTI

	oled_Init();    // Initialize OLED setup, including I/O port PB

	while (1)
	{
		// TODO in main:
		// 1. Poll poten. on PA5
		// 2. Move into ADC and start conversion
		// 	- ADC interrupt will handle DAC and output to circuit
		
		poll_Potentiometer();
		start_ADC();
		
		refresh_OLED(frequency, resistance); // Refresh OLED with frequency and resistance values

		TIM3_Reset(); // Sets TIM3 for ~100 ms to get ~10 frames/sec refresh rate 
//		while(~(TIM3->SR & TIM_SR_UIF_Msk)); // While TIM3 not zero (UIF not set)
		while(TIM3->CNT > 0);
	}

	return 0;
}

// Polls potentiometer (input PA5) and calculates resistance value
void poll_Potentiometer() 
{
	// TODO
}

// Moves resistance value into ADC and starts conversion process
void start_ADC() 
{
	// TODO
}

#pragma GCC diagnostic pop
