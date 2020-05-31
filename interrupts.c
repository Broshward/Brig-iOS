#include "interrupts.h"
#include "main.h"
#include "crontab.h"
//#include "stm32_eval.h"

extern void add_END_to_transmit();

void NMI_Handler(void)
{
	while(1);
}
void HardFault_Handler(void)
{
	sbi(flags,HARD_FAULT);
	//Reset_Handler(void);
	//while(1);
}
void MemManage_Handler(void)
{
	while(1);
}
void BusFault_Handler(void)
{
	while(1);
}
void UsageFault_Handler(void)
{
	while(1);
}
void SVC_Handler(void)
{
	while(1);
}
void DebugMon_Handler(void)
{
	while(1);
}
void PendSV_Handler(void)
{
	while(1);
}
void SysTick_Handler(void)
{
	while(1);
}
void WWDG_IRQHandler(void)
{
	while(1);
}
void PVD_IRQHandler(void)
{
	while(1);
}
void TAMPER_STAMP_IRQHandler(void)
{
	while(1);
}
void RTC_WKUP_IRQHandler(void)
{
//	static uint8_t count=0;
//	if (count) {
//		count=0;
//		GPIOC->BSRR |= 1<<9;
//	}
//	else{
//		count=1;
//		GPIOC->BRR |= 1<<9;
//	}
	while(1);
}
void FLASH_IRQHandler(void)
{
	while(1);
}
void RCC_IRQHandler(void)
{
	while(1);
}
void EXTI0_IRQHandler(void)
{
	while(1);
}
void EXTI1_IRQHandler(void)
{
	while(1);
}
void EXTI2_IRQHandler(void)
{
	while(1);
}
void EXTI3_IRQHandler(void)
{
	while(1);
}
void EXTI4_IRQHandler(void)
{
	while(1);
}
void DMA1_Channel1_IRQHandler(void)
{
	while(1);
}
void DMA1_Channel2_IRQHandler(void)
{
	while(1);
}
void DMA1_Channel3_IRQHandler(void)
{
	while(1);
}
void DMA1_Channel4_IRQHandler(void)
{
	while(1);
}
void DMA1_Channel5_IRQHandler(void)
{
	while(1);
}
void DMA1_Channel6_IRQHandler(void)
{
	while(1);
}
void DMA1_Channel7_IRQHandler(void)
{
	while(1);
}
void ADC1_IRQHandler(void)
{
	ADC1->CR2&= ~ADC_CR2_ADON;
	ADC1->SR &= ~ADC_SR_EOC;
	__WFE();
	while(1);
}
void EXTI9_5_IRQHandler(void)
{
	while(1);
}
void TIM1_BRK_IRQHandler(void)
{
	while(1);
}
void TIM1_UP_IRQHandler(void)
{
	while(1);
}
void TIM1_TRG_COM_IRQHandler(void)
{
	while(1);
}
void TIM1_CC_IRQHandler(void)
{
	while(1);
}
void TIM2_IRQHandler(void)
{
	while(1);
}
void TIM3_IRQHandler(void)
{
	while(1);
}
void TIM4_IRQHandler(void)
{
	if (temperature) temperature += (int32_t)(((uint32_t)ADC1->DR<<20)-temperature)>>4;
	else temperature = ADC1->DR<<20;
	ADC1->CR2 |= ADC_CR2_ADON;
	TIM4->SR =0;
	
	while(1);
}
void I2C1_EV_IRQHandler(void)
{
	while(1);
}
void I2C1_ER_IRQHandler(void)
{
	while(1);
}
void I2C2_EV_IRQHandler(void)
{
	while(1);
}
void I2C2_ER_IRQHandler(void)
{
	while(1);
}
void SPI1_IRQHandler(void)
{
	while(1);
}
void SPI2_IRQHandler(void)
{
	while(1);
}
void USART1_IRQHandler(void)
{
	while(1);
}
void USART2_IRQHandler(void)
{
	while(1);
}
void USART3_IRQHandler(void)
{
	while(1);
}
void EXTI15_10_IRQHandler(void)
{
	while(1);
}
void RTCAlarm_IRQHandler(void)
{
	for (uint8_t i=0; i<sizeof(calls)*8; i++)
		if (calls & (1<<i)){
			//alarm_action(i);
			cron_action(i);
		}
	set_alarm(next_alarm());

	RTC->CRL &= ~RTC_CRL_ALRF;
	EXTI->PR |= 1<<17;
}
void CEC_IRQHandler(void)
{
	while(1);
}
void TIM12_IRQHandler(void)
{
	while(1);
}
void TIM13_IRQHandler(void)
{
	while(1);
}
void TIM14_IRQHandler(void)
{
	while(1);
}
void FSMC_IRQHandler(void)
{
	while(1);
}
void TIM5_IRQHandler(void)
{
	while(1);
}
void SPI3_IRQHandler(void)
{
	while(1);
}
void UART4_IRQHandler(void)
{
	while(1);
}
void UART5_IRQHandler(void)
{
	while(1);
}
void TIM6_DAC_IRQHandler(void)
{
	while(1);
}
void TIM7_IRQHandler(void)
{
	while(1);
}
void DMA2_Channel1_IRQHandler(void)
{
	while(1);
}
void DMA2_Channel2_IRQHandler(void)
{
	while(1);
}
void DMA2_Channel3_IRQHandler(void)
{
	while(1);
}
void DMA2_Channel4_5_IRQHandler(void)
{
	while(1);
}
void DMA2_Channel5_IRQHandler(void)
{
	while(1);
}
