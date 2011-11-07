#ifndef __UTIL_H__
#define __UTIL_H__

#define UNUSED_PARAMETER(x) (x) = (x)

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;

#define NULL    '\0'

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);

void memset(void *buffer, uint8_t ch, uint32_t size);


#define FN_ENTRY()      printf("%s() -->%s:%d\n", __func__, __FILE__, __LINE__)
#define FN_EXIT()      printf("%s() <--%s:%d\n", __func__, __FILE__, __LINE__)
#endif

