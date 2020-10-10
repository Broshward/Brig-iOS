#include "math.h"
#include "main.h"
#include "uart.h"
#include "crontab.h"
#include "spi.h"
#include "adc.h"
#include "dma.h"
//#include "syscalls.h"
//#include "stdlib.h"
//#include <string.h>

uint32_t flags;
uint32_t sys_clock;
uint32_t temperature;
JOURNALING_DATA jdata;
uint16_t ADC_average[12];

const uint32_t sys_time = 1533128400; // Birthtime of BrigðiOS. Historical remains :-))
uint32_t *test_value=(uint32_t*)0x2000080C;

uint32_t recent_time, recent_alarm;

uint32_t clock_frequency_measure();

int main()
{
    RCC->APB1ENR |= RCC_APB1Periph_BKP | RCC_APB1Periph_PWR;
	PWR->CR |= PWR_CR_DBP; //Unblock RTC & BKP register for write access
	RCC->BDCR |= RCC_BDCR_RTCSEL_LSE | RCC_BDCR_RTCEN | RCC_BDCR_LSEON; //
	while(!(RCC->BDCR & RCC_BDCR_LSERDY)); //wait for LSE stable
	RTC->CRH = RTC_CRH_ALRIE;   // Enable "Alarm interrupt" for RTC peripherial
	//RTC->CRH |= RTC_CRH_SECIE;   // Enable "Second interrupt" for RTC peripherial (WakeUP vector (#3))
	//INTERRUPT_ENABLE(3); // Second interrupt come to WakeUp vector (3)
	while(!(RTC->CRL & RTC_CRL_RTOFF)); // wait for write are terminated
//	RTC->CRL |= RTC_CRL_CNF;     // unblock write access for PRL, CNT, DIV  register of RTC
//	RTC->PRLL = 0x7FFF; //RTC preloader for 1 second period increment Timer
//	RTC->CNTH = sys_time>>16;
//	RTC->CNTL = sys_time;
//	RTC->ALRH = *((uint16_t*)&alarm+1);//RTC->ALRH = alarm>>16;
//	RTC->ALRL = *(uint16_t*)&alarm;    //RTC->ALRL = alarm;
//	RTC->CRL &= ~RTC_CRL_CNF;//  for write protect PRL, CNT, DIV
	INTERRUPT_ENABLE(41);//Enable ALARM RTC interrupt in NVIC
	EXTI->IMR |= 1<<17; // RTC_ALARM interrupt enable
	EXTI->RTSR|= 1<<17; // RTC_ALARM rising edge

//	Tables for HPS_Growbox: 
	crontab[0] = "*/15 * * * * * 40003000 VAAAA";	// IWDG->KR = 0xAAAA

//	Test and tuning example
//	crontab[9] = "*/5 * * * * * 40010810=8000";		// GPIOA->BSRR=1<<15
//	crontab[10] = "1-59/5 * * * * * 40010C10=10";	
//	crontab[11] = "2-59/5 * * * * * 40010C14=10";
//	crontab[12] = "3-59/5 * * * * * 40010C10=4";
//	crontab[13] = "4-59/5 * * * * * 40010C14=4,40010814=8000";
//	crontab[14] = "*/2 * * * * * 20000F00=AAAA";

	RCC->CSR |= RCC_CSR_LSION;
	while(!(RCC->CSR & RCC_CSR_LSIRDY));

// IWDG config
	//IWDG->KR = 0xCCCC;
	//IWDG->KR = 0x5555;
	//IWDG->PR = 0b111;
	////DBGMCU->CR |= DBGMCU_IWDG_STOP; // For debug watchdog

RCC->APB2ENR |= RCC_APB2Periph_AFIO;
	AFIO->MAPR |= 0b010<<24; // JTAG off SWD enable

RCC->APB2ENR |= RCC_APB2Periph_GPIOB;
	GPIOB->CRL &= ~((0b1111 << 2*4) | (0b1111 << 4*4) | (0b1111 << 5*4)); //GPIOB 2,4 - output for relay control
	GPIOB->CRL |= (0b0001 << 2*4) | (0b0001 << 4*4) | (0b0001 << 5*4); //GPIOB 2,4 - output for relay control
	GPIOB->ODR = 0; // For reset pullup of GPIOB 4 pin

// Errors and reset flags detects	
	if (RCC->CSR & (RCC_CSR_PINRSTF))
		sbi(flags,PINRST);
	if (RCC->CSR & (RCC_CSR_PORRSTF))
		sbi(flags,PORRST);
	if (RCC->CSR & (RCC_CSR_SFTRSTF))
		sbi(flags,STFRST);
	if (RCC->CSR & (RCC_CSR_IWDGRSTF))
		sbi(flags,IWDGRST);
	if (RCC->CSR & (RCC_CSR_WWDGRSTF))
		sbi(flags,WWDGRST);
	if (RCC->CSR & (RCC_CSR_LPWRRSTF))
		sbi(flags,LPWRRST);
	if (GPIOB->ODR & (1<<2))
		sbi(flags,CHANNEL1_IS_SET);
	if (GPIOB->ODR & (1<<4))
		sbi(flags,CHANNEL2_IS_SET);
	RCC->CSR |= RCC_CSR_RMVF; // Clear reset flags

	
RCC->APB2ENR |= RCC_APB2Periph_GPIOD;
	AFIO->MAPR |= AFIO_MAPR_PD01_REMAP;
	GPIO_conf(GPIOD,0,OUT_10MHZ); //	GPIOD->CRL &= ~((0b1111 << 0*4) | (0b1111 << 1*4)); //GPIOD 0,1 - output for LED
	GPIO_conf(GPIOD,1,OUT_10MHZ); //	GPIOD->CRL |= (0b0001 << 0*4) | (0b0001 << 1*4); //GPIOD 0,1 - output for LED

RCC->APB2ENR |= RCC_APB2Periph_GPIOC;

sys_clock=clock_frequency_measure();
RCC->APB2ENR |= RCC_APB2Periph_USART1; //Включение тактовой USART 
UART_initialization(9600);

//Software_SPI_init();
//SPI_initialization();
//DMA_init();
//ADC_init();

	set_alarm(next_alarm());

// Timer config
	RCC->APB1ENR |= RCC_APB1Periph_TIM4;
	//Timer_fastPWM_ch1(TIM2);
	Timer_PWM_CenterAligned_for_BRIDGE_DCDC_channel34(TIM4);
	GPIO_conf(GPIOB,8,AFIO_PP | OUT_50MHZ);
	GPIO_conf(GPIOB,9,AFIO_PP | OUT_50MHZ);

	RCC->APB2ENR |= RCC_APB2Periph_TIM1;
	Timer_PWM_CenterAligned_for_BRIDGE_DCDC_channel34(TIM1);
	RCC->APB2ENR |= RCC_APB2Periph_GPIOA;
	GPIO_conf(GPIOA,10,AFIO_PP | OUT_50MHZ);
	GPIO_conf(GPIOA,11,AFIO_PP | OUT_50MHZ);


	while(1){
		transmit_uart_buffer();
		recieve_uart_buffer();
		if (RTC->CNTH!=0){				// pseudo protection from unexpected zeroing RTC. (RTC->CNT == 0)
			if (RTC->CNTL!=(uint16_t)recent_time){
				recent_time = RTC->CNTL + (RTC->CNTH<<16);
				recent_alarm= RTC->ALRL + (RTC->ALRH<<16);
				if (bit_is_set(GPIOD->ODR,0))
					sbi(GPIOD->BRR,0);
				else
					sbi(GPIOD->BSRR,0);
			}
		}
		else if (recent_time!=0){
			sbi(flags,TIME_CLEAR);
			RTC->CRL |= RTC_CRL_CNF;     // unblock write access for PRL, CNT, DIV  register of RTC
			RTC->CNTH = recent_time>>16;
			RTC->CNTL = (uint16_t)recent_time;
			RTC->ALRH = recent_alarm>>16;
			RTC->ALRL = (uint16_t)recent_alarm+1;
			RTC->CRL &= ~RTC_CRL_CNF;//  for write protect PRL, CNT, DIV
		}
		if (DMA1->ISR &	DMA1_FLAG_TC1){
			DMA1->IFCR=0xF; //Clear DMA flags
			uint16_t *channels=jdata.ADC_channels; 
			for (int i=0; i<12;i++){
				ADC_average[i] = ((ADC_average[i]<<2)-ADC_average[i] + channels[i])>>2; // (average[i]*3+channels[i])/4  - Average for 32 value filter
				// Open greenhouse  for ventulation!
				//if (i==num && ADC_average[i]>value_max && !(GPIOx->ODR & 1<<y)){
				//	GPIOx->BSRR = 1<<y;
				//	for (uint32_t j=0;j<UINT_MAX;j++); //Pause
				//	GPIOx->BRR = 1<<y;
				//}
				//if (i==num && ADC_average[i]<value_min && (GPIOx->ODR & 1<<y)){
				//	GPIOx->BRR = 1<<y;
				//	for (uint32_t j=0;j<UINT_MAX;j++); //Pause
				//	GPIOx->BRR = 1<<y;
				//}
			}
			if (GPIOC->IDR & (1<<13))
				sbi(flags,POWER_OFF);
			else
				cbi(flags,POWER_OFF);
#ifdef SPI_FLASH
			if (*NSS==0) continue; // SPI is busy 
			uint8_t send_size=sizeof(jdata);
			((uint16_t*)&jdata)[1] = RTC->CNTH;
			((uint16_t*)&jdata)[0] = RTC->CNTL;
			uint32_t flash_cur_addr;
			*((uint16_t*)&flash_cur_addr) = flash_cur_addr_L;
			*((uint16_t*)&flash_cur_addr+1) = flash_cur_addr_H; 
			if (flash_cur_addr==0){ // BKP reset was occur
				sbi(flags,FLASH_CUR_ADDR_is_0);
				flash_cur_addr = search_end_of_data(FLASH_MIN_ADDR, FLASH_MAX_ADDR, send_size);
			}
			else if(flash_cur_addr<FLASH_MIN_ADDR){
				sbi(flags,FLASH_CUR_ADDR_less_MIN);
				flash_cur_addr = search_end_of_data(FLASH_MIN_ADDR, FLASH_MAX_ADDR, send_size);
			}
			jdata.flags = flags;
			PAGE_PROG(flash_cur_addr, (char*)&jdata, send_size);
			flash_cur_addr+=send_size;
			if ((FLASH_MAX_ADDR-flash_cur_addr)<send_size){
				flash_cur_addr = FLASH_MIN_ADDR;
				sbi(flags, FLASH_CUR_ADDR_is_MIN_ADDR);
			}
			READ(flash_cur_addr, spi_buf, send_size);
			for (int i=0 ; i<send_size ; i++){
				if (spi_buf[i]!=0xFF) {
					SECTOR_ER(flash_cur_addr+i);
				}
			}
			flash_cur_addr_L=*((uint16_t*)&flash_cur_addr);
			flash_cur_addr_H=*((uint16_t*)&flash_cur_addr+1); 
#endif
		}
	}

/*	RCC->APB2ENR |= RCC_APB2Periph_ADC1;
	SETMASK(ADC1->SMPR1,ADC_SMPR1_SMP16,0b110);// 111 in SMP16
	ADC1->SQR3 = 16;
	ADC1->CR2 = ADC_CR2_TSVREFE;
	ADC1->CR1 = ADC_CR1_EOCIE;

	RCC->APB1ENR = RCC_APB1Periph_TIM4;
	TIM4->PSC = 100; //100 - 0.8s timer period(65535) for 8 MHz
	TIM4->CR1 = TIM_CR1_OPM;
	TIM4->DIER = TIM_DIER_UIE;
		
	
	INTERRUPT_ENABLE(18);//Enable ADC interrupt in NVIC
	INTERRUPT_ENABLE(30);//Enable TIM4 interrupt
	//INTERRUPT_ENABLE(3);// Enable interrupt for RTC wakeup vector in NVIC
	SCB->SCR |= SCB_SCR_SEVONPEND;

    RCC->APB1ENR |= RCC_APB1Periph_USART2;//Включение тактовой USART (на APB1 шине висит)
    RCC->APB1ENR |= RCC_APB1Periph_SPI2;  //Включение тактовой SPI

*/}


uint32_t clock_frequency_measure()
{
	TIM_TypeDef TIM2_temp, TIM3_temp;
	uint32_t TIM2_clk = RCC->APB1ENR & RCC_APB1Periph_TIM2;
	uint32_t TIM3_clk = RCC->APB1ENR & RCC_APB1Periph_TIM3;
	memcpy(&TIM2_temp,TIM2,sizeof(*TIM2));
	memcpy(&TIM3_temp,TIM3,sizeof(*TIM3));

	RCC->APB1ENR |= RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3;
	SETMASK(TIM2->CR2, TIM_CR2_MMS, 0b010);
	SETMASK(TIM3->SMCR, TIM_SMCR_TS, 0b001);
	SETMASK(TIM3->SMCR, TIM_SMCR_SMS,0b111);
	TIM2->CR1 = TIM_CR1_CEN;
	TIM3->CR1 = TIM_CR1_CEN;

	RTC->CRL &= ~RTC_CRL_SECF;
	while (!(RTC->CRL & RTC_CRL_SECF));
	TIM2->CNT=0;
	TIM3->CNT = 0;
	RTC->CRL &= ~RTC_CRL_SECF;
	while (!(RTC->CRL & RTC_CRL_SECF));
	TIM2->CR1 =0;
	TIM3->CR1= 0;
	uint32_t frequency = (TIM3->CNT<<16)+TIM2->CNT;
	memcpy(TIM2,&TIM2_temp,sizeof(*TIM2));
	memcpy(TIM3,&TIM3_temp,sizeof(*TIM3));
	RCC->APB1ENR &= ~(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3);
	RCC->APB1ENR |= TIM2_clk | TIM3_clk;
	return frequency;
}

