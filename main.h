#include "stm32f10x.h"
#include "core_cm3.h"

uint32_t sys_clock;
uint32_t temperature;

#define SYS_FREQ 24000000
//#define INTERRUPT_PARSER
#define CIRCULAR_PARSER
//#define STRING_PARSER
#define BUF_SIZE 1024;
#define PIPE_ADDRESS   12;
#define UART_SPEED   115200;


#define sbi(reg,num) reg |= (1<<num)
#define cbi(reg,num) reg &= (~((uint32_t)(1<<num)))
#define sbi_32(reg,num) reg |= (1<<num)
#define cbi_32(reg,num) reg &= (~((uint32_t)(1<<num)))
#define bit_is_set(reg,num) reg & (1<<num)
#define bit_is_clear(reg,num) !(reg & (1<<num))

#define find_1(mask) (\
	((mask>>0) & 1) ? 0 : \
	((mask>>1) & 1) ? 1 : \
	((mask>>2) & 1) ? 2 : \
	((mask>>3) & 1) ? 3 : \
	((mask>>4) & 1) ? 4 : \
	((mask>>5) & 1) ? 5 : \
	((mask>>6) & 1) ? 6 : \
	((mask>>7) & 1) ? 7 : \
	((mask>>8) & 1) ? 8 : \
	((mask>>9) & 1) ? 9 : \
	((mask>>10) & 1) ? 10 : \
	((mask>>11) & 1) ? 11 : \
	((mask>>12) & 1) ? 12 : \
	((mask>>13) & 1) ? 13 : \
	((mask>>14) & 1) ? 14 : \
	((mask>>15) & 1) ? 15 : \
	((mask>>16) & 1) ? 16 : \
	((mask>>17) & 1) ? 17 : \
	((mask>>18) & 1) ? 18 : \
	((mask>>19) & 1) ? 19 : \
	((mask>>20) & 1) ? 20 : \
	((mask>>21) & 1) ? 21 : \
	((mask>>22) & 1) ? 22 : \
	((mask>>23) & 1) ? 23 : \
	((mask>>24) & 1) ? 24 : \
	((mask>>25) & 1) ? 25 : \
	((mask>>26) & 1) ? 26 : \
	((mask>>27) & 1) ? 27 : \
	((mask>>28) & 1) ? 28 : \
	((mask>>29) & 1) ? 29 : \
	((mask>>30) & 1) ? 30 : 31)
#define SETMASK(reg,mask,val)	\
	reg &= ~mask;\
	reg |= (val<<find_1(mask));

#define lsi_ON_OFF(sw)	\
  if (sw) cbi_32(RCC->CSR,RCC_CSR_LSION);\
  else sbi_32(RCC->CSR,RCC_CSR_LSION);
#define lse_ON_OFF(sw)	\
  if (sw) cbi_32(RCC->BDCR,RCC_BDCR_LSEON);\
  else sbi_32(RCC->BDCR,RCC_BDCR_LSEON);
#define lse_READY() bit_is_set(RCC->BDCR,RCC_BDCR_LSERDY)
#define INTERRUPT_ENABLE(vec_num) sbi(NVIC->ISER[vec_num / 32],vec_num % 32)
#define INTERRUPT_DISABLE(vec_num) sbi(NVIC->ICER[vec_num / 32],vec_num % 32)
#define INTERRUPT_GENERATE(vec_num) sbi(NVIC->ISPR[vec_num / 32],vec_num % 32)
#define INTERRUPT_CANCEL(vec_num) sbi(NVIC->ICPR[vec_num / 32],vec_num % 32)


