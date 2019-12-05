/*
 * =====================================================================================
 *
 *       Filename:  protocol.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04.09.2012 20:53:57
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */
#include <string.h>
#include <stdlib.h>
#include "protocol.h"
#include "main.h"
#include "term_commands.h"

#define _8to8 &~(DMA_CCR1_PSIZE | DMA_CCR1_MSIZE)
#define _8to16 &~(DMA_CCR1_PSIZE | DMA_CCR1_MSIZE_0) | DMA_CCR1_MSIZE_1
#define _8to32 &~(DMA_CCR1_PSIZE | DMA_CCR1_MSIZE_1) | DMA_CCR1_MSIZE_0
#define _16to16 & ~(DMA_CCR1_PSIZE_0 | DMA_CCR1_MSIZE_0) | (DMA_CCR1_PSIZE_1 | DMA_CCR1_MSIZE_1)
#define _32to32 | DMA_CCR1_PSIZE_0 | DMA_CCR1_MSIZE_0 & ~(DMA_CCR1_PSIZE_1 | DMA_CCR1_MSIZE_1)

void hardware_init(pipe_t *pipe)
{
    RCC->APB2ENR |= RCC_APB2Periph_GPIOA; //Включение тактовой GPIOA
    RCC->APB2ENR |= RCC_APB2Periph_AFIO;  //Включение тактовой AFIO

/*----- USARTx configured as follow:----------------------------------------------
        - BaudRate = 115200 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
	GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

#define USART_ratio(USART_freq,USART_overclock) (SYS_FREQ/USART_freq/USART_overclock) //
#define mantiss(USART_freq,USART_overclock) ((int)SYS_FREQ/(USART_freq)/(USART_overclock))
#define fraqtional(USART_freq,USART_overclock) ((int)(((float)USART_ratio(USART_freq,USART_overclock)-(int)(USART_ratio(USART_freq,USART_overclock)))*USART_overclock)) 
	pipe->arch = 4;
	pipe->USART->BRR = (mantiss(115200,8) << 4) | fraqtional(115200,8);
	pipe->USART->CR1 = USART_CR1_UE | USART_CR1_OVER8 | USART_CR1_TE | USART_CR1_RE;;
	uint8_t temp[pipe->buf_size];
	pipe->buf_rx = temp;
//	pipe->buf_rx=malloc(pipe->buf_size);
	pipe->packet.index = -1; //Т.е. никакого прогресса :))
	pipe->packet.len = 1;
	pipe->buf_rx_index = 0; //индекс принятого байта.
//	pipe->packet.waited_part_len=1;

	RCC->AHBENR |= RCC_AHBPeriph_DMA1; //Включение тактовой ДМА1 (на AHB шине висит)
//приемный буфер
#if defined(CIRCULAR_PARSER) || defined(STRING_PARSER)
		pipe->DMA_Channel_Recieve->CCR &= ~DMA_CCR1_EN;
		pipe->DMA_Channel_Recieve->CPAR=(uint32_t)&(pipe->USART->DR);
		pipe->DMA_Channel_Recieve->CMAR=(uint32_t)pipe->buf_rx;
		pipe->DMA_Channel_Recieve->CNDTR=pipe->buf_size;
		pipe->DMA_Channel_Recieve->CCR=DMA_CCR1_MINC | DMA_CCR1_CIRC | DMA_CCR1_EN ;
#endif
#ifdef INTERRUPT_PARSER
		DMA1_Channel6->CCR &= ~DMA_CCR1_EN;
		DMA1_Channel6->CPAR=(uint32_t)&(pipe->USART->DR);
		DMA1_Channel6->CMAR=(uint32_t)&pipe->packet.dev_addr;
		DMA1_Channel6->CNDTR=1;
		DMA1_Channel6->CCR=DMA_CCR1_MINC | DMA_CCR1_EN | DMA_CCR1_TCIE;

		NVIC->ISER[0]= (uint32_t)1<< DMA1_Channel6_IRQn;
		NVIC->IP[DMA1_Channel6_IRQn]=10<<4;
#endif
#ifdef LLMDB
#endif
#ifdef TERMINAL_STRING
#endif
		pipe->USART->CR3 |= USART_CR3_DMAR; //turn on DMA request for USART RX
//приемный буфер

//передающий буфер
		pipe->DMA_Channel_Transmit->CCR &= ~DMA_CCR1_EN;
		pipe->DMA_Channel_Transmit->CPAR=(uint32_t)&(pipe->USART->DR);
		pipe->DMA_Channel_Transmit->CNDTR=0;
		pipe->DMA_Channel_Transmit->CCR=DMA_CCR1_MINC | DMA_CCR1_DIR;

		pipe->USART->CR3 |= USART_CR3_DMAT; //turn on DMA request for USART TX
//передающий буфер

		pipe->DMA_Channel_Data_Copy->CCR &= ~DMA_CCR1_EN; //Канал для копирования массивов в памяти
		pipe->DMA_Channel_Data_Copy->CCR=DMA_CCR1_MINC | DMA_CCR1_PINC | DMA_CCR1_MEM2MEM | DMA_CCR1_PL_0;

}

#ifdef CIRCULAR_PARSER
/* 1. First byte is DEVICE ADDRESS ( pipe->pipe_address)
 * 2. Command byte (select performed command). Description below.
 * 2.5 ? Byte-parameter of command ? (May be abscent)
 * 3. Some command specific bytes. Usually 0...261 bytes, but can be more.
 * $. Last is the CheckSum (CS) byte. Bitwise XOR operation of all bytes of packet
 */ 
int packet_parser(pipe_t *pipe)
{
	uint8_t simul=0;//for simulation process
	//recieve_bytes_count = 
	if (pipe->packet.end >= pipe->buf_size)
		pipe->packet.end = 0; //To Begin of buf_rx
	if (simul==0 && ((pipe->packet.end) > (pipe->buf_size - pipe->DMA_Channel_Recieve->CNDTR)))
		return 0; // Next byte is recieve
	
	if (pipe->packet.index==-1) // First (ADDRESS) byte recieved
	{
			if (pipe->buf_rx[pipe->buf_rx_index]!=pipe->pipe_address) // Packet is not mine
			{
				pipe->packet.end++;
				return 0;
			}
			else //Packet is mine :)
			{
				pipe->packet.beg = pipe->packet.index = pipe->packet.end+1; // The begin index of packet without address of device, (as known) !!!!
				pipe->packet.end += 2; // for recieve COMMAND and CS (for minimal command)
				pipe->packet.progress++; // Speak function about command recieve
				return 1;
			}
	}

	if (pipe->packet.index==1) //Command are recieved
	{
		if (pipe->buf_rx[pipe->packet.index] == WRITE_ARRAY) // 
			pipe->packet.end += 4 + pipe->buf_rx[pipe->packet.index++] +1; //Len of array address + len of array data (Current recieve byte is len of data array) + len of CheckSum
		else if (pipe->buf_rx[pipe->packet.index] == READ_ARRAY)
			pipe->packet.end += 4 +1; // Len of address of begin of data array + len of CS
	}
		
	if (pipe->packet.index >= pipe->packet.end)
	{
		uint8_t CS = pipe->pipe_address;
		for (int i=pipe->packet.beg;i != pipe->packet.end;i++)
		{
			if (i==pipe->buf_size)
				i=0;
			CS ^= pipe->buf_rx[i];
		}
		pipe->packet.index = -1;
		if (!CS)
		{
			switch pipe->buf_rx[pipe->packet.beg
		}
		return 2;
	pipe->packet.index++;
	}
}
#endif


#ifdef INTERRUPT_PARSER
int packet_parser()
{
	static int progress = 0; //Т.е. никакого прогресса :))
	
	DMA1_Channel6->CCR &= ~DMA_CCR1_EN;
	switch (progress) 
	{
		case 0:
				if (packet.dev_addr==DEVICE_ADDRESS)
					{
						DMA1_Channel6->CMAR = (uint32_t)&(packet.command);
						DMA1_Channel6->CNDTR = 2; //два байта команды
						progress++;
					}
				else
				{
					DMA1_Channel6->CMAR = (uint32_t)&(packet.dev_addr);
					DMA1_Channel6->CNDTR = 1;
				}
		break;
		case 1:
				DMA1_Channel6->CMAR = (uint32_t)&(packet.addr_len);
				DMA1_Channel6->CNDTR = 1;
			progress++;
		break;
		case 2:
				progress++;
				if (packet.addr_len==0)
				{
					packet_parser();
				}
				else
				{
					DMA1_Channel6->CMAR = (uint32_t)&(packet.addr);
					DMA1_Channel6->CNDTR = packet.addr_len;
				}
		break;
		case 3:
				DMA1_Channel6->CMAR = (uint32_t)&(packet.data_len);
				DMA1_Channel6->CNDTR = 1;
			progress++;
		break;
		case 4:
				progress++;
				if (packet.data_len==0)
				{
					packet_parser();
				}
				else
				{
				DMA1_Channel6->CMAR = (uint32_t)&(packet.data);
				DMA1_Channel6->CNDTR = packet.data_len;
				}
		break;
		case 5:
				DMA1_Channel6->CMAR = (uint32_t)&(packet.CS);
				DMA1_Channel6->CNDTR = 1;
			progress++;
		break;
		case 6:
				DMA1_Channel6->CMAR = (uint32_t)&(packet.dev_addr);
				DMA1_Channel6->CNDTR = 1;

				uint8_t CS=0;
				for (int i=0;i<sizeof(packet_t);i++)
					CS ^= ((uint8_t*)&packet)[i];
				progress=0;
				if (!CS)
				{
				//	strncpy(pipe->exec.command, packet->command, packet->command_len);
				//	int i;
				//	for (i=0; i<packet->addr_len; i++)
				//		pipe->exec.addr += packet->addr[i]>>8*i;
				//	for (i=0; i<packet->data_len; i++)
				//		pipe->exec.data[i] = packet->data[i];
				//	pipe->exec.data[i] = -1;
				//	exec_command(exec);
				}
		break;
		default:
		break;
	}
	DMA1_Channel6->CCR |= DMA_CCR1_EN;
}
#endif

int llmdb_parser()
{
	return 0;
}

#ifdef STRING_PARSER
int string_parser()
{
	static int first_char=0;
	static int current_char=0;
	static char string[256];

	if (((current_char) == BUF_SIZE - DMA1_Channel6->CNDTR)  )
		return 0;

	for (;current_char!=BUF_SIZE-DMA1_Channel6->CNDTR;current_char++)
	{
		if (current_char==BUF_SIZE)
			current_char=0;
		if (buf_rx[current_char]=='\n')
		{
			for (int i=0;first_char!=current_char;first_char++)
			{
				string[i] = buf_rx[first_char+1];
				if (first_char==BUF_SIZE)
					first_char=0;
				i++;
			}
//			first_char=current_char;
			if (string[0]!=0)
			{
					//strncpy(pipe->exec.command, packet->command, packet->command_len);
					//int i;
					//for (i=0; i<packet->addr_len; i++)
					//	pipe->exec.addr += packet->addr[i]>>8*i;
					//for (i=0; i<packet->data_len; i++)
					//	pipe->exec.data[i] = packet->data[i];
					//pipe->exec.data[i] = -1;
				DMA1_Channel7->CCR &= ~DMA_CCR1_EN;
				DMA1_Channel7->CMAR=(uint32_t)"Ok";
				DMA1_Channel7->CNDTR=4;
				DMA1_Channel7->CCR |= DMA_CCR1_EN;
			}
		}
	}
}
#endif


void exec_command(pipe_t *pipe)
{
	switch (pipe->exec.command)
	{
		case 0:
		break;
		case 1:
		break;
		case WRITE_UINT8_T: //write uint8_t
			**(uint8_t **)pipe->exec.data = (uint8_t)pipe->exec.data[sizeof(uint8_t*)];
			//memcpy(*(uint8_t **)pipe->exec.data,((uint8_t *)pipe->exec.data)+sizeof(uint8_t *),sizeof(uint8_t));
		break;
		case WRITE_UINT16_T: //write uint16_t
			**(uint16_t **)pipe->exec.data = (uint16_t)pipe->exec.data[sizeof(uint16_t*)];
			//memcpy(*(uint16_t **)pipe->exec.data,((uint16_t *)pipe->exec.data)+sizeof(uint16_t *),sizeof(uint16_t));
		break;
		case WRITE_UINT32_T: //write uint32_t
			**(uint32_t **)pipe->exec.data = (uint32_t)pipe->exec.data[sizeof(uint32_t*)];
			//memcpy(*(uint32_t **)pipe->exec.data,((uint32_t *)pipe->exec.data)+sizeof(uint32_t *),sizeof(uint32_t));
		break;
		case WRITE_UINT32_T_ARRAY: //write array of uint32_t (32 to 32)
			pipe->DMA_Channel_Transmit->CCR &= ~DMA_CCR1_EN;
			pipe->DMA_Channel_Data_Copy->CCR &= ~(DMA_CCR1_MSIZE_0 | DMA_CCR1_PSIZE_0);
			pipe->DMA_Channel_Data_Copy->CCR |= (DMA_CCR1_MSIZE_1 | DMA_CCR1_PSIZE_1);
			pipe->DMA_Channel_Data_Copy->CMAR=*((uint32_t*)(pipe->exec.data));
			pipe->DMA_Channel_Data_Copy->CPAR=(uint32_t)((uint32_t*)pipe->exec.data+sizeof(uint32_t*));
			pipe->DMA_Channel_Data_Copy->CNDTR=(pipe->exec.data_count-pipe->arch)/4; // count Of 32-BIT data
			pipe->DMA_Channel_Data_Copy->CCR |= DMA_CCR1_EN;
		break;
		case WRITE_UINT8_T_ARRAY: //write array of uint32_t (32 to 32)
			pipe->DMA_Channel_Transmit->CCR &= ~DMA_CCR1_EN;
			pipe->DMA_Channel_Data_Copy->CCR &= ~(DMA_CCR1_MSIZE | DMA_CCR1_PSIZE);
			pipe->DMA_Channel_Data_Copy->CMAR=*((uint8_t*)(pipe->exec.data));
			pipe->DMA_Channel_Data_Copy->CPAR=(uint32_t)((uint8_t*)pipe->exec.data+sizeof(uint32_t*));
			pipe->DMA_Channel_Data_Copy->CNDTR=pipe->exec.data_count-pipe->arch; // count Of 32-BIT data
			pipe->DMA_Channel_Data_Copy->CCR |= DMA_CCR1_EN;
		break;
//		case 15: //write array of uint16_t (16 to 32)
//			pipe->DMA_Channel_Data_Copy->CCR &= ~DMA_CCR1_EN;
//			pipe->DMA_Channel_Data_Copy->CMAR=*((uint32_t*)(pipe->exec.data));
//			pipe->DMA_Channel_Data_Copy->CPAR=(uint32_t)((uint32_t*)pipe->exec.data+sizeof(uint32_t*));
//			pipe->DMA_Channel_Data_Copy->CNDTR=pipe->exec.data_count;
//			pipe->DMA_Channel_Data_Copy->CCR |= DMA_CCR1_EN;
//			pipe->exec.addr=(uint32_t)pipe->exec.data;
//		break;
		case READ_UINT8_T: //read uint8_t
			pipe->DMA_Channel_Transmit->CCR &= ~DMA_CCR1_EN;
			pipe->DMA_Channel_Transmit->CMAR=*((uint8_t*)(pipe->exec.data));
			pipe->DMA_Channel_Transmit->CNDTR=1;
			pipe->DMA_Channel_Transmit->CCR |= DMA_CCR1_EN;
		break;
		case READ_UINT16_T: //read uint16_t
			pipe->DMA_Channel_Transmit->CCR &= ~DMA_CCR1_EN;
			pipe->DMA_Channel_Transmit->CMAR=*((uint16_t*)(pipe->exec.data));
			pipe->DMA_Channel_Transmit->CNDTR=2;
			pipe->DMA_Channel_Transmit->CCR |= DMA_CCR1_EN;
		break;
		case READ_UINT32_T: //read uint32_t
			pipe->DMA_Channel_Transmit->CCR &= ~DMA_CCR1_EN;
			pipe->DMA_Channel_Transmit->CMAR=*((uint32_t*)(pipe->exec.data));
			pipe->DMA_Channel_Transmit->CNDTR=4;
			pipe->DMA_Channel_Transmit->CCR |= DMA_CCR1_EN;
		break;
		case READ_UINT32_T_ARRAY: //read array of uint8_t
			pipe->DMA_Channel_Transmit->CCR &= ~DMA_CCR1_EN;
			pipe->DMA_Channel_Transmit->CMAR=*((uint8_t*)(pipe->exec.data));
			pipe->DMA_Channel_Transmit->CNDTR=(uint32_t)(pipe->exec.data[pipe->arch]);
			pipe->DMA_Channel_Transmit->CCR |= DMA_CCR1_EN;
		break;
	}
}

