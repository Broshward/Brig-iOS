//#define SPI_FLASH // For write journaling to external flash processing
#define WRIPSISB  // For new compact and fuctional data exchange protocol
//#define DEBUG		// For send to UART information about alarm time and action number


#include "stm32f10x.h"
#include "core_cm3.h"

typedef struct  {
	uint32_t time __attribute__ ((__packed__)); //(( aligned(8) ))
	uint16_t ADC_channels[12] __attribute__ ((__packed__));
	uint32_t flags __attribute__ ((__packed__));
	const char end;
} JOURNALING_DATA; // Data for journaling in external flash
extern JOURNALING_DATA jdata;

extern uint32_t sys_clock;
extern uint32_t temperature;
extern uint32_t flags;
extern uint16_t ADC_average[12];
extern JOURNALING_DATA jdata;

#define TIMEH_of_settime BKP->DR1 // Time of last setting time MSB
#define TIMEL_of_settime BKP->DR2 // LSB
#define flash_cur_addr_H BKP->DR3 
#define flash_cur_addr_L BKP->DR4 

// Number of bit accorded certainf flag in flags register
#define HARD_FAULT 0
#define TIME_CLEAR 1
#define CHANNEL1_IS_SET 2
#define CHANNEL2_IS_SET 3
#define FLASH_CUR_ADDR_is_0 4 // probably BKP reset was occur
#define FLASH_CUR_ADDR_is_MIN_ADDR 5 // probably Flash memory is ended
#define FLASH_is_ended 6
#define FLASH_CUR_ADDR_less_MIN 7

#define POWER_OFF 8 // 5V Power is absent
#define PINRST 31
#define PORRST 30
#define STFRST 29
#define IWDGRST 28
#define WWDGRST 27
#define LPWRRST 26 //0x4000000

#define FLASH_MIN_ADDR 4096 // First 4096 bytes reserved for user data
#define FLASH_MAX_ADDR 0x3FFFFF // Last byte of flash array (Pm25LQ032C). Specify for your flash chip MAX byte address please

#define SYS_FREQ 24000000
//#define INTERRUPT_PARSER
#define CIRCULAR_PARSER
//#define STRING_PARSER
#define BUF_SIZE 1024;
#define PIPE_ADDRESS   12;
#define UART_SPEED   115200;


#define bit_band_of(reg,bit) (uint32_t*)(((uint32_t)(reg)&0x60000000) | (0x02000000) | (((uint32_t)(reg)&0xFFFFF)<<5) | (bit<<2))

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

//GPIO
// Pinout confs:

#define ANALOG 0b0000
#define FLOATING_INP 0b0100 // Reset state
#define PULLUPDOWN_INP 0b1000
#define OD			0b0100
#define AFIO_PP 	0b1000
#define AFIO_OD 	0b1100
#define OUT_10MHZ	0b0001
#define OUT_2MHZ	0b0010
#define OUT_50MHZ	0b0011


#define GPIO_conf(GPIO,num,CONF) \
	*((uint32_t*)GPIO+num/8) &= ~(0b1111 << (num%8)*4); \
	*((uint32_t*)GPIO+num/8) |= ((CONF) << (num%8)*4);

/*#define GPIO_confs(GPIO,...,CONF) \
//	for (int i=0; i<)
//	*((uint32_t*)GPIO+num/8) &= ~(0b1111 << (num%8)*4); \
	*((uint32_t*)GPIO+num/8) |= ((CONF) << (num%8)*4);
*/

//#define 
#define Timer_PWM_CenterAligned_for_BRIDGE_DCDC_channel34(TIM) \
	TIM->CR1 =  TIM_CR1_ARPE | TIM_CR1_CEN; \
	SETMASK(TIM->CR1,TIM_CR1_CMS,0b11);\
	TIM->CCMR2 = (0b110<<4) | (0b110<<12) | TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE;\
	TIM->CCER = TIM_CCER_CC4E | TIM_CCER_CC4P | TIM_CCER_CC3E ;\
	TIM->BDTR = TIM_BDTR_MOE;

#define Timer_fastPWM_ch1(TIM) \
	TIM->CR1 =  TIM_CR1_ARPE | TIM_CR1_CEN; \
	TIM->ARR = 100;\
	TIM->CCR1 = 10;\
	TIM->BDTR = TIM_BDTR_MOE;
/*	TIM->CCMR1 = (0b110<<4)  | TIM_CCMR1_OC1PE;\
	TIM->CCER =  TIM_CCER_CC1E ;\*/
//void PWM_halfbridge_conf(TIM_TypeDef *TIM,)
