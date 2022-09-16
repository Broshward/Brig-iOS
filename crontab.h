#include "stm32f10x.h"
#include <stddef.h>
#include "strings.h"
#include "uart.h"

//extern void add_to_transmit(uint8_t num);
//extern void add_to_transmit_uint16(uint16_t num);
//extern void add_to_transmit_uint32(uint32_t num);
//extern void add_to_transmit_str(uint8_t *str);
//extern void add_END_to_transmit();

extern int32_t localtime; // Local time relatively UTC in seconds, because exist fractional hour time zones

typedef struct {
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t mon;
	uint8_t year;
	uint8_t century;
} TIME;

extern char *crontab[32]; //For more than 32 regular expression records are more 32-bit wide of calls variable needed. Or other algorithm needed (not bit mask), for example array of alarmed numbers
extern uint32_t calls; // The bit mask alarmed cron tabs 32 bit wide

void ShowLed(int8_t T);
uint8_t dec_num_parse(char **str);
uint32_t hex_num_parse(char **str);
void utc_to_time(TIME *tm, uint32_t utc);
uint32_t time_to_utc(TIME *tm);
uint32_t next_time(char *str);
uint32_t next_alarm();
void set_alarm(uint32_t alarm);
void cron_action(uint8_t number);

//typedef struct {
//	char* time;
//	uint32_t* dest_addr;
//	uint32_t* src_addr;
//	uint32_t*  value; 
//} CRON_TAB;

//void cron_add_tab(char* tab_content);
//void *malloc(size_t size);
//uint32_t *hexvalue_parser(char *str, char *end_str);
//void time_parser(char *str);
//void alarm_action(uint8_t number);
