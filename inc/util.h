#ifndef __UTIL_H__
#define __UTIL_H__

#define UNUSED_PARAMETER(x) (x) = (x)


typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;

typedef enum __bool
{
    false = 0,
    true = 1,
}bool;
#define NULL    '\0'

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);

void memset(void *buffer, uint8_t ch, uint32_t size);

#include "display.h"

#define FN_ENTRY()     printf("%s() -->%s:%d\n", __func__, __FILE__, __LINE__)
#define FN_EXIT()      printf("%s() <--%s:%d\n", __func__, __FILE__, __LINE__)

#define LOG()                       printf("%s:%d>"  "\n", __FILE__, __LINE__)

#ifdef DEBUG
#define LOG_INFO(format, args...)   printf("%s:%d>" format "\n", __FILE__, __LINE__, ## args)
#else
#define LOG_INFO(format, args...)  
#endif
#define LOG_WARN(format, args...)   printf("%s:%d>" format "\n", __FILE__, __LINE__, ## args)
#define LOG_ERROR(format, args...)  printf("%s:%d>" format "\n", __FILE__, __LINE__, ## args)



uint32_t strcpy(char *dest, char *src);
uint32_t strlen(char *str);

#endif

