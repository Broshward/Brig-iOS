#include "main.h"
#include "sspi.h"
#include "stm32f10x.h"

//---------------------------------------------------------------------------
// Software SPI :)))  Only for master mode! 
void SPI_initialization()
{
	// GPIOB 12 - NSS Pin, out for master 
	// GPIOB 13 - SCK pin, out for master
	// GPIOB 14 - MISO pin, input 
	// GPIOB 15 - MOSI pin, out 
	//
	sbi(GPIOB->ODR,12);
	GPIOB->CRH &= ~( (0xF<<4*(12-8)) | (0xF<<4*(13-8)) | (0xF<<4*(14-8)) | (0xF<<4*(15-8)) );
	GPIOB->CRH |= ( (0b0011<<4*(12-8)) | (0b0011<<4*(13-8)) | (0b0100<<4*(14-8)) | (0b0011<<4*(15-8)) );
	NSS = (uint32_t*)( 0x42000000 + ((uint32_t)&(GPIOB->ODR)-0x40000000)*32 + 12*4);
	SCK = (uint32_t*)( 0x42000000 + ((uint32_t)&(GPIOB->ODR)-0x40000000)*32 + 13*4);
	MISO= (uint32_t*)( 0x42000000 + ((uint32_t)&(GPIOB->ODR)-0x40000000)*32 + 14*4);
	MOSI= (uint32_t*)( 0x42000000 + ((uint32_t)&(GPIOB->ODR)-0x40000000)*32 + 15*4);
//	CPOL=0; // 0 - for rising clock polarity, 1 for falling
//	if (CPOL) *SCK=1;
DR_tx_bit_band = (uint32_t*)(((uint32_t)&DR_tx-0x20000000)*32 + 0x22000000);
DR_rx_bit_band = (uint32_t*)(((uint32_t)&DR_rx-0x20000000)*32 + 0x22000000);
DR_max = DR_tx_bit_band +32;
}

char SPI_rw(char DR_tx)
{
	char DR_rx;
	for (int i=7 ; i>=0 ; i--){
		if (DR_tx & 1<<i) *MOSI=1;
		else *MOSI=0;
//		if (CPOL) { // Uncomment for clock polarity adjust (its still more slowly)
//			*SCK=0;
//			*SCK=1;
//		}
//		else {
			*SCK=1;
			*SCK=0;
//		}
		DR_rx = (DR_rx<<1) | *MISO;
	}
	return DR_rx;
}

void _SPI_rw(void) //Fastest function, but in STM32F100C8 SRAM bit banding makes "Cannot access memory" error and Hardfault reset occurs.... Why???
{
	uint32_t *rx=DR_rx_bit_band;
	uint32_t *tx=DR_tx_bit_band;
	while (tx<DR_max){
		*MOSI = *tx;
		*SCK=1;
		*SCK=0;
		*rx = *MISO;
		rx++;
		tx++;
	}
}

