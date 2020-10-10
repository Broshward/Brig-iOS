#include "main.h"
#include "spi.h"
#include "strings.h"
#include "stm32f10x.h"

uint32_t *NSS; // Bit banding address for NSS pin
char spi_buf[256];

void SPI_initialization()
{
	RCC->APB1ENR |= RCC_APB1Periph_SPI2;  
	SPI2->CR2 = SPI_CR2_SSOE;
	SPI2->CR1 = SPI_CR1_MSTR | SPI_CR1_SPE;// | SPI_CR1_CPOL | SPI_CR1_CPHA; 
	//SETMASK(SPI2->CR1, SPI_CR1_BR, 0b10);

	// GPIOB 12 - NSS Pin, out for master (standalone)
	// GPIOB 13 - SCK pin, out alternate function
	// GPIOB 14 - MISO pin, input standalone
	// GPIOB 15 - MOSI pin, out alternate func
	//sbi(GPIOB->ODR,12);
	GPIOB->CRH &= ~( (0xF<<4*(12-8)) | (0xF<<4*(13-8)) | (0xF<<4*(14-8)) | (0xF<<4*(15-8)) );
	GPIOB->CRH |= ( (0b0011<<4*(12-8)) | (0b1011<<4*(13-8)) | (0b0100<<4*(14-8)) | (0b1011<<4*(15-8)) );
	NSS = bit_band_of(&GPIOB->ODR,12);
//#define NSS bit_band_of(&GPIOB->ODR,12) // Alternate bit band pin initialization
	*NSS=1;
//	INTERRUPT_ENABLE(35);
}

void USART_as_SPI_initializathion() // Only for LSB first
{
	USART1->BRR = 0b10000; // Minimum of divider
	USART1->CR2 = USART_CR2_CLKEN; 
	USART1->CR1 = USART_CR1_OVER8 | USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;

	AFIO->MAPR &= ~AFIO_MAPR_USART1_REMAP;
	GPIOA->CRH &= ~((0xF << 4*(10-8)) | (0xF << 4*(9-8)) | (0xF << 4*(8-8)));
	GPIOA->CRH |= (0b0100 << 4*(10-8)) | (0b1010 << 4*(9-8)) | (0b1010 << 4*(10-8));
	//INTERRUPT_ENABLE(37);
}

void send_cmd(uint8_t cmd)
{
	*NSS=0;
	while(!(SPI2->SR & SPI_SR_TXE));
	SPI2->DR = cmd;
}

void send_addr(uint32_t addr)
{
	for(int i=2;i>=0;i--){
		while(!(SPI2->SR & SPI_SR_TXE));
		SPI2->DR = *((uint8_t*)&addr+i);
	}
}

void send_data(char *buf, uint8_t count)
{
	for (int i=0;i<count;i++){
		while(!(SPI2->SR & SPI_SR_TXE));
		SPI2->DR = buf[i];
	}
	*NSS=1;
}

uint8_t get_value()
{
	uint8_t value;
	while((!(SPI2->SR & SPI_SR_TXE)) || SPI2->SR & (SPI_SR_BSY | SPI_SR_RXNE)) //Clear RX buffer register
		value=SPI2->DR;
	SPI2->DR = 0; 
	while(!(SPI2->SR & SPI_SR_RXNE));
	value = SPI2->DR;
	*NSS=1;
	return value;
}

char* get_data(char *buf, uint8_t count)
{
	while((!(SPI2->SR & SPI_SR_TXE)) || SPI2->SR & (SPI_SR_BSY | SPI_SR_RXNE)) //Clear RX buffer register
		buf[0]=SPI2->DR;

	for (int i=0;i<count;i++){
		SPI2->DR = 0; 
		while(!(SPI2->SR & SPI_SR_RXNE));
		buf[i] = SPI2->DR;
	}
	*NSS=1;
	return buf;
}

char* SPI_rw(char* spi_buf, uint8_t command_len, uint8_t data_len)
{
	*NSS=0;
	int i;
	for(i=0;i<command_len;i++){
		SPI2->DR = spi_buf[i];
		while(SPI2->SR & SPI_SR_TXE);
	}
	while(SPI2->SR & SPI_SR_RXNE) //Clear RX buffer register
		spi_buf[i]=SPI2->DR;

	for(i=0;i<data_len;i++){
		SPI2->DR = 0;
		while(!(SPI2->SR & SPI_SR_RXNE));
		spi_buf[i]=SPI2->DR;
	}
	*NSS=1;
	return spi_buf;
}

char* J_ID_read(char *buf)
{
	send_cmd(0b10011111);
	return get_data(buf,3);
	//spi_buf[0] = 0b10011111;
	//return SPI_rw(spi_buf,1,3);
}

char* RDID(char *buf)
{
	send_cmd(0b10101011);
	send_addr(0);
	return get_data(buf,3);
	//spi_buf[0]=0b10101011;
	//return SPI_rw(spi_buf,4,3);
}

char* RDMDID(char *buf)
{
	send_cmd(0x90);
	send_addr(0);
	return get_data(buf,3);
	//spi_buf[0]=0x90;
	//return SPI_rw(spi_buf,4,3);
}

void WREN()
{
	while(RDSR() & 1);
	send_cmd(0b00000110);
	*NSS=1;
	//spi_buf[0]=0b00000110;
	//SPI_rw(spi_buf,1,0);
}

uint8_t RDSR()
{
	send_cmd(0b00000101);
	return get_value();
//	spi_buf[0]=0b00000101;
//	return SPI_rw(spi_buf,1,1);
}

void WRSR(char value)
{
	WREN();
	send_cmd(0b00000001);
	send_data(&value,1);
	//spi_buf[0]=0b00000001;
	//spi_buf[1]=value;
	//SPI_rw(spi_buf,2,0);
}

char* READ(uint32_t addr, char *buf, uint8_t count)
{
	while(RDSR() & 1);
	send_cmd(0b00000011);
	send_addr(addr);
	return get_data(buf, count);
	//spi_buf[0]=0b00000011;
	//spi_buf[1]=*((uint8_t*)&addr+2);//(uint8_t)(addr>>16);
	//spi_buf[2]=*((uint8_t*)&addr+1);//(uint8_t)(addr>>8);
	//spi_buf[3]=*((uint8_t*)&addr+0);//(uint8_t)(addr);
	//return SPI_rw(spi_buf,4,count);
}

void PAGE_PROG(uint32_t addr, char *buf, uint8_t count)
{
	WREN();
	send_cmd(0b00000010);
	send_addr(addr);
	send_data(buf,count);
}

void SECTOR_ER(uint32_t addr)
{
	WREN();
	send_cmd(0b11010111);
	send_addr(addr);
	*NSS=1;
}

void BLOCK_ER(uint32_t addr)
{
	WREN();
	send_cmd(0b11011000);
	send_addr(addr);
	*NSS=1;
}

void CHIP_ER(uint32_t addr)
{
	WREN();
	send_cmd(0b11000111);
	*NSS=1;
}

/* This function searchs first clear data_size of data in flash array */
uint32_t search_end_of_data(uint32_t first, uint32_t last, uint32_t data_size) 
{
	send_cmd(0b00000011);//READ
	send_addr(first);
	while((!(SPI2->SR & SPI_SR_TXE)) || SPI2->SR & (SPI_SR_BSY | SPI_SR_RXNE)) //Clear RX buffer register
		if (SPI2->DR);

	int count=0;
	for (int i=first;i<last;i++){
		if (count == data_size) {
			count = i-count;
			break;
		}
		SPI2->DR = 0; 
		while(!(SPI2->SR & SPI_SR_RXNE));
		if (SPI2->DR == 0xFF) count++;
		else count=0;
	}
	*NSS=1;
	if (count<FLASH_MIN_ADDR){
		count = FLASH_MIN_ADDR;
		sbi(flags, FLASH_CUR_ADDR_is_MIN_ADDR);
		SECTOR_ER(count);
	}
	return count;
}
