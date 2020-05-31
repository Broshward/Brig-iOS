#include "main.h"
#include "dma.h"
#include "adc.h"

void DMA_init()
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	// First channel for sequence of ADC channels
	DMA1_Channel1->CNDTR = 12;
	DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
	DMA1_Channel1->CMAR = (uint32_t)jdata.ADC_channels;
	#define MSIZE 0b01<<10	// For 16 bit transaction
	#define PSIZE 0b01<<8	// For 16 bit transaction
	#define MINC 1<<7		//Memory increment
	#define CIRC 1<<5		//Circular mode
	DMA1_Channel1->CCR = MSIZE | PSIZE | MINC | CIRC | 1; // Last 1 for ENable
}


//-----------DMA1---------------------------
	//---- DMA for transmit buffer-----------------
//	DMA1_Channel4->CPAR = (uint32_t)&USART1->DR;
//	DMA1_Channel4->CMAR = (uint32_t)transmit_buf;
//	DMA1_Channel4->CNDTR = sizeof(transmit_buf);
//	DMA1_Channel4->CCR = DMA_CCR4_MINC | DMA_CCR4_DIR;

	//---- DMA for recieve to buffer-----------------
//	DMA1_Channel5->CPAR = (uint32_t)&USART1->DR;
//	DMA1_Channel5->CMAR = (uint32_t)recieve_buf;
//	DMA1_Channel5->CNDTR = sizeof(recieve_buf);
//	DMA1_Channel5->CCR = DMA_CCR4_MINC | DMA_CCR4_CIRC | DMA_CCR4_EN;
//	USART1->CR3 = USART_CR3_DMAT | USART_CR3_DMAR; 
//	USART1->SR = 0;

