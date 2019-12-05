#include "stm32f10x.h"
#include <stddef.h>

uint16_t _strlen(char *str);
char *_strchr(char *str, int c);
char *strncpy(char *dest, char *src, size_t n);
char *strblank(char *str, uint16_t count);
char *strnonblank(char *str);
uint16_t strcountchar(char *str, int c);
uint16_t strncountchar(char *str, int c, uint16_t count);
char *strnchr(char *str, uint16_t len, int c, uint16_t count);
uint32_t strntouint(char *str, uint16_t count, uint8_t base);
int32_t powint(int32_t num, int8_t pow);
int8_t _strncmp(char *s1, char *s2, uint8_t n);
