#include "main.h"
#include "adc.h"

void ADC_init()
{
	//GPIO for analog ADC channels
RCC->APB2ENR |= RCC_APB2Periph_GPIOA;
	GPIOA->CRL = 0; // PA0..PA7 makes analog inputs
RCC->APB2ENR |= RCC_APB2Periph_GPIOB;
	GPIOB->CRL &= ~0xFF; // PB0, PB1 - analog inputs

	RCC->APB2ENR |= RCC_APB2Periph_ADC1;
	ADC1->CR1 = ADC_CR1_SCAN;
	ADC1->CR2 = ADC_CR2_TSVREFE | ADC_CR2_DMA | ADC_CR2_CAL;
	//SETMASK(ADC1->SQR1,ADC_SQR1_L,11);
	ADC1->SQR1 = (12-1)<<20; // -1 because 0 is a One (1) conversion
	ADC1->SQR3 = 0 | 1<<5*1 | 2<<5*2 | 3<<5*3 | 4<<5*4 | 5<<5*5;
	ADC1->SQR2 = 6 | 7<<5*1 | 8<<5*2 | 9<<5*3 | 16<<5*4 | 17<<5*5;
	ADC1->SMPR1 = 0b010<<3*8 | 0b010;
	ADC1->SMPR2 = 0b00010010010010010010010010010010;

	RCC->CFGR |= RCC_CFGR_ADCPRE; // ADC clock divider /8

	ADC1->CR2 |= ADC_CR2_ADON;
	while (ADC1->CR2 & ADC_CR2_CAL);
}

