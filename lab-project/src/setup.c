/* Contains initialization functions for each port */

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

/* Clock prescaler for TIM2 timer: no prescaling */
#define myTIM2_PRESCALER ((uint16_t)0x0000)

/* Maximum possible setting for overflow */
#define myTIM2_PERIOD ((uint32_t)0xFFFFFFFF)

/*** Call this function to boost the STM32F0xx clock to 48 MHz ***/
void SystemClock48MHz( void )
{
	// Disable the PLL
	RCC->CR &= ~(RCC_CR_PLLON);
	
	// Wait for the PLL to unlock
	while (( RCC->CR & RCC_CR_PLLRDY ) != 0 );
	
	// Configure the PLL for 48-MHz system clock
	RCC->CFGR = 0x00280000;
	
	// Enable the PLL
	RCC->CR |= RCC_CR_PLLON;
	
	// Wait for the PLL to lock
	while (( RCC->CR & RCC_CR_PLLRDY ) != RCC_CR_PLLRDY );
	
	// Switch the processor to the PLL clock source
	RCC->CFGR = ( RCC->CFGR & (~RCC_CFGR_SW_Msk)) | RCC_CFGR_SW_PLL;
	
	// Update the system with the new clock frequency
	SystemCoreClockUpdate();
}

void GPIOA_Init()
{
	/* Enable clock for GPIOA peripheral */
	// Relevant register: RCC->AHBENR
	/* Set bit 17 IOPAEN to enable port A clock */
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	/* Configure PA2 as input */
	// Relevant register: GPIOA->MODER
	/* Clear bits [5:4] to ensure PA2 is set for input */
	GPIOA->MODER &= ~(GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1); // &= 0b 1100 1111

	/* Ensure no pull-up/pull-down for PA2 */
	// Relevant register: GPIOA->PUPDR
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR4_1  | GPIO_PUPDR_PUPDR5_1 ); // &= 0b 1100 1111
}

void TIM2_Init()
{
	/* Enable clock for TIM2 peripheral */
	// Relevant register: RCC->APB1ENR
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	/* Configure TIM2: buffer auto-reload, count up, stop on overflow,
	 * enable update events, interrupt on overflow only */
	// Relevant register: TIM2->CR1
	TIM2->CR1 |= (TIM_CR1_ARPE | ~TIM_CR1_DIR | TIM_CR1_OPM | ~TIM_CR1_UDIS | TIM_CR1_URS);

	/* Set clock prescaler value */
	TIM2->PSC = myTIM2_PRESCALER;
	/* Set auto-reloaded delay */
	TIM2->ARR = myTIM2_PERIOD;

	/* Update timer registers */
	// Relevant register: TIM2->EGR
	TIM2->EGR |= TIM_EGR_UG;

	/* Assign TIM2 interrupt priority = 0 in NVIC */
	// Relevant register: NVIC->IP[3], or use NVIC_SetPriority
	NVIC_SetPriority(TIM2_IRQn, 0);

	/* Enable TIM2 interrupts in NVIC */
	// Relevant register: NVIC->ISER[0], or use NVIC_EnableIRQ
	NVIC_EnableIRQ(TIM2_IRQn);

	/* Enable update interrupt generation */
	// Relevant register: TIM2->DIER
	TIM2->DIER |= TIM_DIER_UIE;
}

void EXTI_Init()
{
	/* Map EXTI2 line to PA2 */
	// Relevant register: SYSCFG->EXTICR[0]
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PA;

	/* EXTI2 line interrupts: set rising-edge trigger */
	// Relevant register: EXTI->RTSR
	EXTI->RTSR |= EXTI_RTSR_TR2;

	/* Unmask interrupts from EXTI2 line */
	// Relevant register: EXTI->IMR
	EXTI->IMR |= EXTI_IMR_MR2;

	/* Assign EXTI2 interrupt priority = 0 in NVIC */
	// Relevant register: NVIC->IP[2], or use NVIC_SetPriority
	NVIC_SetPriority(EXTI2_3_IRQn, 0);

	/* Enable EXTI2 interrupts in NVIC */
	// Relevant register: NVIC->ISER[0], or use NVIC_EnableIRQ
	NVIC_EnableIRQ(EXTI2_3_IRQn);
}
