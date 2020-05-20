#include <stdint.h>

void add_END_to_transmit();
void add_to_transmit(uint8_t num);
void add_to_transmit_uint16(uint16_t num);
void add_to_transmit_uint32(uint32_t num);
void add_to_transmit_str(char *str);
void add_to_transmit_data(uint8_t count, uint8_t *data);

void cmd_perform(char *str);

void transmit_uart_buffer();
void recieve_uart_buffer();

void UART_initialization(uint32_t sysclock);
