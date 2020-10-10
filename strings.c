#include "strings.h"

uint16_t strncountchar(char *str, int c, uint16_t count)
{
	uint16_t i=0;
	char *end = str+count;
	while(1){
		str=strnchr(str,end-str,c,1);
		if (str==NULL) return i;
		str++;
		i++;
	}
}
uint16_t strcountchar(char *str, int c)
{
	//uint16_t count=0;
	//while(str=_strchr(str,c)!=NULL)
	//	count++;
	//return count;
	return 0;
}

char *strncpy(char *dest, char *src, size_t n)
{
	for (size_t i=0; i<n; i++)
		dest[i] = src[i];
	dest[n]='\0';
	return dest;
}

char *strnonblank(char *str)
{
	while(*str==' ' || *str=='\t' || *str=='\0'){
		if (*str=='\0')
			return NULL;
		str++;
	}
	return str;
}

char *strblank(char *str, uint16_t count)
{
	uint16_t i=1;
	if (count==0) return str;
	while(1){
		if ((*str==' ') || (*str=='\t') || (*str=='\0')) {
			if (i==count) return str;
			else i++;
		}
		else
		if (*str=='\0') return NULL;
		str++;
	}
}

uint16_t _strlen(char *str)
{
	uint16_t len=0;
	while(1){
		if (*(str+len)=='\0') return len;
		len++;
	}
}

char *_strchr(char *str, int c)
{
	while(1){
		if (*str==c) return str;
		if (*str=='\0') return NULL;
		str++;
	}
}
char *strnchr(char *str, uint16_t len, int c, uint16_t count)
{
	uint16_t i=0, j=1;
	if (count==0) return str;
	while(i<len){
		if (*str==c) {
			if (j==count) return str;
			else j++;
		}
		if (*str=='\0') return NULL;
		str++;
		i++;
	}
	return NULL;
}

uint32_t strntouint(char *str, uint16_t count, uint8_t base)
{
	uint32_t result=0;
	for (uint16_t i=0; i<=count; i++)
		result += (str[count-i]-'0'<10 ? str[count-i]-'0' : str[count-i]-'a'+10 )*powint(base,i);
	return result;
}

int32_t powint(int32_t num, int8_t pow) 
{
	int32_t result=1;
	for (int i=0;i<pow;i++)
		result *= num;
	return result;
}

int8_t _strncmp(char *s1, char *s2, uint8_t n){
	for (uint8_t i=0;i<n;i++){
		if (s1[i]!=s2[i])
			return s1[i]-s2[i];
	}
	return 0;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	for (size_t i=0; i<n; i++)
		((char*)dest)[i]=((char*)src)[i];
	return dest;
}
