#include "main.h"
#include "uart.h"
#include "stm32f10x.h"
#include "core_cm3.h"
#include "strings.h"
#include "crontab.h"

uint8_t transmit_buf[256];
uint8_t transmit_queue_index=0;
uint8_t receive_buf[256];

//----- Byte staffing protocol------------------
#define END 0
#define END_change 0xFD
#define END_change_change 0xFC

uint8_t transmited=0;
uint8_t received=0;

void cmd_perform(char *str)
{
	if (!_strncmp(str,"Rb",2)){ //Read byte(uint8_t) from given memory address
		uint8_t* addr=*(uint8_t**)(str+2);
		add_to_transmit_str("b:");
		add_to_transmit(*addr);
		add_END_to_transmit();
	}
	if (!_strncmp(str,"Rh",2)){  //Read half(uint16_t, short) from given memory address
		uint16_t* addr=*(uint16_t**)(str+2);
		add_to_transmit_str("h:");
		add_to_transmit_uint16(*addr);
		add_END_to_transmit();
	}
	if (!_strncmp(str,"Rw",2)){  //Read word(uint32_t) from given memory address
		uint32_t* addr=*(uint32_t**)(str+2);
		add_to_transmit_str("w:");
		add_to_transmit_uint32(*addr);
		add_END_to_transmit();
	}
	if (!_strncmp(str,"Rs",2)){  //Read string from given memory address
		add_to_transmit_str("s:");
		add_to_transmit_str(*(char**)(str+2));
		add_END_to_transmit();
	}
	if (!_strncmp(str,"Rd",2)){  //Read count of data array[count] from given memory address 		
		uint8_t count=*(uint8_t*)(str+2);
		uint8_t* addr=*(uint8_t**)(str+3);
		add_to_transmit_str("d:");
		for (int i=0 ; i<count ; i++)
			add_to_transmit(*(addr+i));
		add_END_to_transmit();
	}

//------------------- Please ADD here data for custom description------------------
	// descript for custom[1,2,3,4..n] string transmit throught corresponding 
	// R1, R2, R3, R4 ... Rn requests
//------------------- Please ADD here data for custom description------------------
	
//	if (!_strncmp(str,"Wc",sizeof("Wc")-1)){ //Write char to given memory address
//		char* addr=str+sizeof("Wc");
//		add_to_transmit(*addr);
//	}
	
	if (!_strncmp(str,"CR",2)){ // Execute crontab compatible string
		str += 2;
		uint32_t *dest=0,*src=0,value=0,temp=0;
		while(*(str-1)!='\0'){
//			if (dest==0) dest = (uint32_t*)hex_num_parse(&str);
//			else if 
			switch (*str++) { // For old string format
				case 'D':
					dest = (uint32_t*)hex_num_parse(&str);
				break;
				case 'S':
					src = (uint32_t*)hex_num_parse(&str);
				break;
				case 'V':
					value = hex_num_parse(&str);
				break;
				case ',':
				case 0:
					if (dest){
						if (src)
							*dest = *src;
						else
							*dest = value;
					}
					else if (src){
						//add_to_transmit_str("d:");
						add_to_transmit_uint32(*src);
						add_END_to_transmit();
					}
					dest=0;
					src=0;
				break;
				default:
				break;
			}
		}
	}
// Special commands
	if (!_strncmp(str,"RT",sizeof("RT")-1)){ //Read time(system) UTC
		add_to_transmit_str("TM:");
		add_to_transmit_uint16(RTC->CNTH);
		add_to_transmit_uint16(RTC->CNTL);
		add_END_to_transmit();
	}
	if (!_strncmp(str,"ST",sizeof("ST")-1)){ //Set time(system) UTC
		const uint8_t base = sizeof("ST")-1;
		while(!(RTC->CRL & RTC_CRL_RTOFF)); // wait for write are terminated
		RTC->CRL |= RTC_CRL_CNF;     // unblock write access for PRL, CNT, DIV  register of RTC
		RTC->CNTH = ((uint8_t)str[base]<<8) + (uint8_t)str[base+1];
		RTC->CNTL = ((uint8_t)str[base+2]<<8)+(uint8_t)str[base+3];
		RTC->CRL &= ~RTC_CRL_CNF;//  for write protect PRL, CNT, DIV
		add_to_transmit_str("OK");
		add_END_to_transmit();
		while(!(RTC->CRL & RTC_CRL_RTOFF)); // wait for write are terminated
		set_alarm(next_alarm());
		RCC->CSR |= 1<<24; //Clear all reset flags
//		GPIOB->BRR=0xFFFF;
	}
}

void add_END_to_transmit()
{
	transmit_buf[transmit_queue_index++]=END;
}

void add_to_transmit(uint8_t num)
{
	switch (num) {
		case END:
			transmit_buf[transmit_queue_index++]=END_change;
			transmit_buf[transmit_queue_index++]=END_change;
			break;
		case END_change:
			transmit_buf[transmit_queue_index++]=END_change;
			transmit_buf[transmit_queue_index++]=END_change_change;
			break;
		default:
			transmit_buf[transmit_queue_index++]=num;
			break;
	}
}

void add_to_transmit_uint16(uint16_t num)
{
	add_to_transmit(num>>8);
	add_to_transmit((uint8_t)num);
}

void add_to_transmit_uint32(uint32_t num)
{
	for(uint8_t i=3;i<4;i--)
		add_to_transmit((uint8_t)(num>>(8*i)));
}

void add_to_transmit_data(uint8_t count, uint8_t *data)
{
	for(uint8_t i=0;i<count;i++)
		add_to_transmit(data[i]);
}

void add_to_transmit_str(char *str)
{
	while(*str){
		add_to_transmit(*str);
		str++;
	}
}

void transmit_uart_buffer()
{
	if (transmit_queue_index!=transmited) // Transmit UART buffer
		if (USART1->SR & USART_SR_TC){
			USART1->DR = transmit_buf[transmited];
			transmited++;
		}
}

void recieve_uart_buffer()
{
	if (USART1->SR & USART_SR_RXNE){ // Recieve UART character to recieve buffer
		receive_buf[received] = USART1->DR;
		if (receive_buf[received] == END){
			cmd_perform((char *)receive_buf);
			received = 0;
		}
		else if ((receive_buf[received] == END_change) && (receive_buf[received-1] == END_change))
			receive_buf[received-1] = END;
		else if ((receive_buf[received] == END_change_change) && (receive_buf[received-1] == END_change))
			receive_buf[received-1] = END_change;
		else received++;
	}
}

void UART_initialization(uint32_t baudrate)
{
	uint16_t ratio = sys_clock/baudrate;
	if (ratio<16) {
		USART1->CR1 |= USART_CR1_OVER8;
		USART1->BRR = ((ratio<<1) & (~0b1111)) | (ratio & 0b111); // (ratio/8<<4) | (ratio%8)
	}
	else {
		USART1->CR1 &= ~USART_CR1_OVER8;
		USART1->BRR = ratio;
	}
	USART1->CR1 |= USART_CR1_UE	| USART_CR1_TE | USART_CR1_RE;
	AFIO->MAPR |= AFIO_MAPR_USART1_REMAP;

	GPIOB->CRL = (0b0100 << 7*4) | (0b1010 << 6*4);
	//SETMASK(GPIOB->CRL, GPIO_CRL_CNF6|GPIO_CRL_MODE6, 0b1010); 
	//SETMASK(GPIOB->CRL, GPIO_CRL_CNF7|GPIO_CRL_MODE7, 0b0100); 
//	RCC->APB2ENR |= RCC_APB2Periph_GPIOA;
//	SETMASK(GPIOA->CRH, GPIO_CRH_CNF10|GPIO_CRH_MODE10, 0b0100); 
//	SETMASK(GPIOA->CRH, GPIO_CRH_CNF9|GPIO_CRH_MODE9, 0b1010); 
	//INTERRUPT_ENABLE(37);
}

