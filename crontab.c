#include "main.h"
#include "crontab.h"

//	uint8_t sec[60];
//	uint8_t min[60];
//	uint8_t hour[24];
//	uint8_t day[31];
//	uint8_t month[12];
//	uint8_t weekday[7];
//
//uint8_t *times[6]={sec,min,hour,day,month,weekday};

//char crontab_memory[2*1024];
int32_t localtime = 3*3600;

void cron_action(uint8_t number)
{
#ifdef DEBUG
add_to_transmit_str("ALR:");
add_to_transmit_data((uint8_t*)&RTC->ALRL, 2); // LSB first
add_to_transmit_data((uint8_t*)&RTC->ALRH, 2);
add_END_to_transmit();
add_to_transmit_str("ACT:");
add_to_transmit(number);
add_END_to_transmit();
#endif
	char *str = strblank(crontab[number],6)+1;
	uint32_t *dest=0,*src=0,value=0;
	while(*(str-1)!='\0'){
		if (!dest) dest = (uint32_t*)hex_num_parse(&str);
		switch (*str++) { 
			case '*':
			case 'S':
				src = (uint32_t*)hex_num_parse(&str);
			break;
			case '=':
			case 'V':
				value = hex_num_parse(&str);
			break;
			case 'F':
				// Add function calling at given address
			break;
			case ',':
			case ';':
			case 0:
				if (dest){
					if (src)
						*dest = *src;
					else
						*dest = value;
				}
				dest=0;
				src=0;
			break;
			default:
			break;
		}
	}
}

//void alarm_action(uint8_t number) //deprecated
//{
////add_to_transmit_str("ACT:");
////add_to_transmit(number);
////add_END_to_transmit();
////	uint32_t *dest = crontab[number]->dest_addr; 
////	uint32_t *src = crontab[number]->src_addr; 
////	uint32_t *value = crontab[number]->value;
////	if (dest == 0 || (src==0 && value==0)) return;
////	uint8_t dest_len = src!=0 ? src-dest : value-dest;
////	uint8_t src_len  = src!=0 ? (value!=0 ? value-src : dest_len) : 0;
////	uint8_t value_len= dest_len - src_len; 
////	
////	uint8_t i=0;
////	for (; i<src_len ; i++)
////		*(uint32_t*)(dest[i]) = *(uint32_t*)(src[i]);
////	for (uint8_t j=0; j<value_len; j++)
////		*(uint32_t*)(dest[i+j]) = value[j];
//}

inline void set_alarm(uint32_t alarm)
{
	RTC->CRL |= RTC_CRL_CNF;     // unblock write access for PRL, CNT, DIV  register of RTC
	RTC->ALRH = *((uint16_t*)&alarm+1);//RTC->ALRH = alarm>>16;
	RTC->ALRL = *(uint16_t*)&alarm;    //RTC->ALRL = alarm;
	RTC->CRL &= ~RTC_CRL_CNF;//  for write protect PRL, CNT, DIV
}

/* Return next time of all alarms in CRONTAB 1*/
uint32_t next_alarm()
{
	uint8_t i=0; 
	uint32_t next_alrm=0xFFFFFFFF;
	calls=0; // global alarms flags cleared
	while (1){
		uint32_t temp;
		if (crontab[i])
			temp = next_time(crontab[i]);
		else {
//add_to_transmit_str("NEXT:");
//add_to_transmit_uint32(next_alrm);
//add_END_to_transmit();
			return next_alrm; 
		}
		if (next_alrm>temp){
			next_alrm=temp;
			calls = 1<<i;
		}
		else if (next_alrm==temp) calls |= 1<<i;
		i++;
	}
}

uint8_t max_field(TIME *tm, uint8_t field);
/* Return next time of current regular expression string */
uint32_t next_time(char *str)
{
	uint32_t curr_time = (RTC->CNTH<<16)+RTC->CNTL;
//add_to_transmit_str("TM:");
//add_to_transmit_uint32(curr_time);
//add_END_to_transmit();
	TIME tm;
	utc_to_time(&tm, curr_time);
	uint8_t alarms_choice[60];
	uint8_t max=12; //First month are check (field=4)
	uint8_t beg_num=255,end_num=255,temp_num=255;
	uint8_t index=0,field=4; //field=4 for month checked, !!! YEAR not checked !!!
	char *save_str = str;
	str = strblank(save_str,field)+1;// +1 because field after space symbol ' ' 
	while(1){ //(field!=0xFF){  --- The BREAK is below
		if ((uint8_t)(*str-'0')<10) //if (*str>='0' && *str<='6') // Number in the field finded
			beg_num = end_num = dec_num_parse(&str);
		if ((*str=='-') || (*str=='*')){
			if (*str=='-'){
				str++;
				end_num = dec_num_parse(&str);				
			}
			else{
				str++;
				if (field==4 || field == 3)
					beg_num=1; // For 'day','month' field. Day or month cannot be zero.
				else
					beg_num=0;
				end_num=max;
			}
			if (*str=='/'){
				str++;
				temp_num = dec_num_parse(&str);
			}
			else
				temp_num = 1;
			for (uint8_t i=beg_num;i<=((end_num<=max) ? end_num : max);i+=temp_num)
				alarms_choice[index++] = i;
			beg_num=255;
		}
		if (*str==','){
			str++;
			if (beg_num<=max)
				alarms_choice[index++] = beg_num;
		}
		if (*str==' '){
			str++; // ??? what for ???
			if (beg_num<=max)
				alarms_choice[index++] = beg_num;
			//find next alarm time in alarms_choice
			uint8_t temp=0xFF;
			for (uint8_t i=0;i<index;i++){
				if ((alarms_choice[i] < temp) && (alarms_choice[i] >= ((uint8_t*)&tm)[field]))
					temp=alarms_choice[i];	
			}
			if (temp==0xFF) {
				((uint8_t*)&tm)[field] = 0xFF;//max+1; //Value more then maximum for next field ++; ((Current field) > max)
				while (((uint8_t*)&tm)[field] > max) {
					field++;
					((uint8_t*)&tm)[field]++; 
					max=max_field(&tm, field);
				}
				for (uint8_t i=0;i<field;i++) 
					((uint8_t*)&tm)[i] = 0;
				field++; // For current field crontab check because further "field--" operation
			}
			else if (temp > ((uint8_t*)&tm)[field]) { 
				((uint8_t*)&tm)[field] = temp;	
				for (uint8_t i=0;i<field;i++) 
					((uint8_t*)&tm)[i] = 0;
			}
			field--; 
			max=max_field(&tm, field);
			if (field==0xFF) break; 
			str = field ? strblank(save_str,field)+1 : save_str;
			index=0;
		}
	}
	return time_to_utc(&tm);
}

uint8_t max_field(TIME *tm, uint8_t field)
{
	uint8_t max;
	switch (field) {
		case 0:
			max = 59; // Max second's value
			break;
		case 1:
			max = 59;// Max minute's value
			break;
		case 2:
			max = 23;// Max hour's value
			break;
		case 3:// Max day's value
			if (tm->mon == 2){
				if (tm->year % 4)
					max = 28;
				else
					max = 29;
			}
			else if (((tm->mon % 2) && (tm->mon < 8)) || ((!(tm->mon % 2)) && (tm->mon > 7)))
				max = 31;
			else
				max = 30;
			break;
		case 4:
			max = 12;// Max month's value
			break;
		case 5:
			max = 99;// Max year's value
			break;
	}
	return max;
}

uint32_t time_to_utc(TIME *tm)
{
	uint32_t utc=0;
	for (int i=1970;i<tm->year+tm->century*100;i++)
		if (i%4) utc += (365*86400);
		else utc += (366*86400);
	for (int i=1;i<tm->mon;i++)
		if (i == 2) {
			if (tm->year % 4)
				utc += 28*86400;
			else
				utc += 29*86400;
		}
		else if (((i % 2) && (i < 8)) || ((!(i % 2)) && (i > 7)))
			utc += 31*86400;
		else
			utc += 30*86400;
	utc+=((tm->day-1)*86400);
	utc+=(tm->hour*3600);
	utc+=(tm->min*60);
	utc+=tm->sec;
	utc -= localtime; // For right UTC time without your time zone settings
	return utc;
}

void utc_to_time(TIME *tm, uint32_t utc)
{
	utc += localtime; // For you right time zone
	tm->century=19;
	tm->year=70;
	tm->mon =1;
	tm->day =1;
	tm->hour=0;
	tm->min =0;
	tm->sec =0;
	uint32_t year_sec = 365*86400;
	while (utc>year_sec) {
		tm->year++;
		utc-=year_sec;
		if (tm->year % 4)
			year_sec = 365*86400;
		else
			year_sec = 366*86400;
		if (tm->year==100){
			tm->year=0;
			tm->century++;
		}
	}

	uint32_t mon_sec = 31*86400;
	while (utc>mon_sec){
		tm->mon++;
		utc-=mon_sec;
		if (tm->mon == 2){
			if (tm->year % 4)
				mon_sec = 28*86400;
			else
				mon_sec = 29*86400;
		}
		else if (((tm->mon % 2) && (tm->mon < 8)) || ((!(tm->mon % 2)) && (tm->mon > 7)))
			mon_sec = 31*86400;
		else
			mon_sec = 30*86400;
	}
	tm->day += (utc/86400);
	utc %= 86400;
	tm->hour += (utc/3600);
	utc %= 3600;
	tm->min += (utc/60);
	utc %= 60;
	tm->sec = utc;
}

//void cron_add_tab(char* tab_content)
//{
////	int i=0;
////	for(;i<100;i++){
////		if (crontab[i]==0) break;
////		if (i==99) return; // Add error message answer memory ended
////	}
////	crontab[i] = malloc(sizeof(CRON_TAB));
////	char *time_end = strblank(tab_content,6);
////	//crontab[i]->time = malloc(time_end-tab_content+1);
////	crontab[i]->time = malloc(_strlen(tab_content));
////	strncpy(crontab[i]->time, tab_content, _strlen(tab_content));
////
////	tab_content = strnonblank(time_end);
////	if (tab_content==NULL) {
////		crontab[i]->dest_addr=0;
////		return;
////	}
//////	action_parser(tab_content);
////	char *end_str = strblank(tab_content, 1);
////	crontab[i]->dest_addr = hexvalue_parser(tab_content,end_str);
////	crontab[i]->src_addr=0;
////	crontab[i]->value=0;
////	tab_content = strnonblank(end_str);
////	while(*tab_content!='\0'){
////		end_str = strblank(tab_content, 1);
////		if (*tab_content=='V'){
////			tab_content++;
////			crontab[i]->value = hexvalue_parser(tab_content,end_str);
////		}
////		else
////			crontab[i]->src_addr = hexvalue_parser(tab_content,end_str);
////		if (*end_str=='\0') break;
////		tab_content = strnonblank(end_str);
////	}
//}

//void action_parse(char *str)
//{
//	uint32_t *dest,*src,value;
//	while(*(str-1)!='\0'){
//		switch (*str++) {
//			case 'D':
//				dest = (uint32_t*)hex_num_parse(&str);
//			break;
//			case 'S':
//				src = (uint32_t*)hex_num_parse(&str);
//			break;
//			case 'V':
//				value = hex_num_parse(&str);
//			break;
//			case ',':
//			case 0:
//				if (dest && src)
//					*dest = *src;
//				if (dest && value)
//					*dest = value;
//			break;
//			default:
//			break;
//		}
//	}
//}
//void time_parser(char *str)
//{
//	uint8_t max=60;
//	uint8_t beg_num=255,end_num=255,temp_num=255;
//	
//	uint8_t index=0,field=0;
//	while(*str!='\0'){
//		if ((uint8_t)(*str-'0')<6) //if (*str>='0' && *str<='6')
//			beg_num = end_num = dec_num_parse(&str);
//		if ((*str=='-') || (*str=='*')){
//			if (*str=='-'){
//				str++;
//				end_num = dec_num_parse(&str);				
//			}
//			else{
//				str++;
//				beg_num=0;
//				end_num=max;
//			}
//			if (*str=='/'){
//				str++;
//				temp_num = dec_num_parse(&str);
//			}
//			else
//				temp_num = 1;
//			for (uint8_t i=beg_num;i< ((end_num<=max) ? end_num : max);i+=temp_num)
//				times[field][index++] = i;
//			beg_num=255;
//		}
//		if (*str==','){
//			str++;
//			if (beg_num<=max)
//				times[field][index++] = beg_num;
//		}
//		if (*str==' '){
//			str++;
//			if (beg_num<=max)
//				times[field][index++] = beg_num;
//			times[field][index++] = 0;
//			field++;
//			index=0;
//			switch (field) {
//				case 1:
//					max = 60;
//					break;
//				case 2:
//					max = 24;
//					break;
//				case 3:
//					max = 31;
//					break;
//				case 4:
//			max = 12;					break;
//				case 5:
//					max = 7;
//					break;
//			}
//		}
//	}
//}

//void sort(uint8_t array,uint8_t len)
//{
////	uint8_t buf=array[0];
////	for (uint8_t i=0;i<len;i++)
////		if (buf > array[i])
////			buf = array[i];
//}

uint32_t hex_num_parse(char **str)
{
	uint32_t temp_num=0; // -1 more better value for this
	while(1){
		uint8_t dig = **str;
		if ((dig>='0') && (dig<='9'))
			dig -= '0';
		else if (dig>='a' && dig<='f')
			dig -= ('a'-10);
		else if (dig>='A' && dig<='F')
			dig -= ('A'-10);
		else break;
		temp_num *= 16;
		temp_num += dig;
		(*str)++;
	}
	return temp_num;
}

uint8_t dec_num_parse(char **str)
{
	uint8_t temp_num=0;
	while(**str>='0' && **str<='9'){
		temp_num *= 10;
		temp_num += (**str - '0');
		(*str)++;
	}
	return temp_num;
}

//uint32_t *hexvalue_parser(char *str, char *end_str)
//{
//	uint16_t dest_count = strncountchar(str, ',', end_str-str)+1; 
//	uint32_t *value_array = malloc(sizeof(uint32_t)*dest_count);
//	for (int j=0;j<dest_count;j++){
//		value_array[j] = hex_num_parse(&str);//strntouint(str,end_num-str-1,16);
//		str++;
//	}
//	return value_array;
//}

//void *malloc(size_t size)
//{
//	static void *last = crontab_memory;
//	if (size==0) return NULL;
//	last += size;
//	return last-size;
//}


