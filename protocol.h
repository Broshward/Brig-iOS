/*
 * =====================================================================================
 *
 *       Filename:  protocol.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05.09.2012 18:40:35
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */
#include "stm32f10x.h"

/*packet description:
Address of slave device(server)			1
Command to server						1
	command for any address transaction provides server for information about address and data width for transaction 
Count of transmit data
 */
typedef struct {
//	uint8_t command_len;
	int8_t command;
	uint8_t data_len;
	uint8_t data[256];
	uint8_t CS;
	int16_t index;
	uint16_t beg;
	uint16_t end;
	uint8_t progress;
	uint16_t len; //index of end of packet
} packet_t;

typedef struct {
	int16_t command;
	uint8_t data[256];
	uint8_t data_count;
} execute_t;

typedef struct {
	uint8_t arch; // Bus width (in bytes) for microcontroller or embedded microprocessor
	uint8_t pipe_address;
	uint32_t uart_speed;
	uint8_t uart_overclock; //Количество выборок для одного бита UART
	uint16_t buf_size;
	uint8_t *buf_rx;
	uint16_t buf_rx_index; //индекс принятого байта.
	packet_t packet;
	execute_t exec;
	USART_TypeDef *USART;
	DMA_Channel_TypeDef *DMA_Channel_Recieve;
	DMA_Channel_TypeDef *DMA_Channel_Transmit;
	DMA_Channel_TypeDef *DMA_Channel_Data_Copy;
} pipe_t;

execute_t exec;

int packet_parser(pipe_t *pipe);
int llmdb_parser();
int string_parser();
void hardware_init(pipe_t *pipe);
void exec_command(pipe_t *pipe);
//void packet_parser_1();
