// Contains initialization functions for each port and timer 
// used, as well as EXTI and ADC/DAC

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

#define TIM2_PRESCALER ((uint16_t)0x0000)  // Clock prescaler for TIM2 timer: no prescaling
#define TIM2_PERIOD ((uint32_t)0xFFFFFFFF) // Maximum possible setting for overflow

// Call this function to boost the STM32F0xx clock to 48 MHz
void SystemClock48MHz( void )
{
	RCC->CR &= ~(RCC_CR_PLLON);                            // Disable the PLL
	while (( RCC->CR & RCC_CR_PLLRDY ) != 0 );             // Wait for the PLL to unlock
	
	RCC->CFGR = 0x00280000;                                // Configure the PLL for 48-MHz system clock
	RCC->CR |= RCC_CR_PLLON;                               // Enable the PLL
	while (( RCC->CR & RCC_CR_PLLRDY ) != RCC_CR_PLLRDY ); // Wait for the PLL to lock
	
	RCC->CFGR = ( RCC->CFGR & (~RCC_CFGR_SW_Msk)) | RCC_CFGR_SW_PLL; // Switch the processor to the PLL clock source
	
	SystemCoreClockUpdate(); // Update the system with the new clock frequency
}

void ADC_Init()
{
	RCC->APB2ENR |= RCC_APB2ENR_ADCEN; // RCC clock enable for ADC

	ADC1->CHSELR = ADC_CHSELR_CHSEL5; // Select channel 5 (PA5)
	ADC1->SMPR = 0b111; // Set sampling rate to max

	NVIC_EnableIRQ(ADC1_IRQn);

	ADC1->CR |= ADC_CR_ADEN; // ADC enable

	while((ADC1->ISR & ADC_ISR_ADRDY_Msk) == 0); // Wait until ADC ready

	ADC1->CR |= ADC_CR_ADSTART; // Start ADC
}

void DAC_Init()
{
	RCC-> APB1ENR |= RCC_APB1ENR_DACEN; // RCC clock enable for DAC
	DAC->CR |= DAC_CR_EN1; // Enable DAC out for PA4
}

void GPIOA_Init() // TODO: Setup all other PA pins
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN; // Enable clock for GPIOA peripheral

	GPIOA->MODER |= (0b11 << GPIO_MODER_MODER4_Pos) | (0b11 << GPIO_MODER_MODER5_Pos); // Configure PA dir - all in except PA4, PA5

	ADC_Init();
	// DAC_Init();
}

void TIM2_Init()
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enable clock for TIM2 peripheral

	// Configure TIM2: buffer auto-reload, count up, stop on overflow,
	// enable update events, interrupt on overflow only
	TIM2->CR1 |= (TIM_CR1_ARPE | ~TIM_CR1_DIR | TIM_CR1_OPM | ~TIM_CR1_UDIS | TIM_CR1_URS);

	TIM2->PSC = TIM2_PRESCALER;     // Set clock prescaler value
	TIM2->ARR = TIM2_PERIOD;        // Set auto-reloaded delay

	NVIC_SetPriority(TIM2_IRQn, 0); // Assign TIM2 interrupt priority = 0 in NVIC
	NVIC_EnableIRQ(TIM2_IRQn);      // Enable TIM2 interrupts in NVIC

	TIM2->EGR |= TIM_EGR_UG;        // Update timer registers
	TIM2->DIER |= TIM_DIER_UIE;     // Enable update interrupt generation
}

void TIM3_Init()
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;     // Enable clock for TIM3 peripheral
    TIM3->CR1 |= TIM_CR1_DIR | TIM_CR1_OPM; // Downcounter mode, OPM enabled (reset CEN when 0 reached)
}

void TIM3_Reset()
{
    TIM3->SR &= ~TIM_SR_UIF_Msk;       // Clear UIF
    TIM3->CNT = SystemCoreClock * 0.1; // 100ms at 48MHz
    TIM3->CR1 |= TIM_CR1_CEN;    	   // Start timer
}

void EXTI_Init()
{
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PA; // Map EXTI2 line to PA2

	EXTI->RTSR |= EXTI_RTSR_TR2; // EXTI2 line interrupts: set rising-edge trigger
	EXTI->IMR |= EXTI_IMR_MR2;   // Unmask interrupts from EXTI2 line
	
    NVIC_SetPriority(EXTI2_3_IRQn, 0); // Assign EXTI2 interrupt priority = 0 in NVIC
	NVIC_EnableIRQ(EXTI2_3_IRQn);      // Enable EXTI2 interrupts in NVIC
}
