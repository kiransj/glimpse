#ifndef __UTIL_H__
#define __UTIL_H__

#define UNUSED_PARAMETER(x) (x) = (x)

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

#define NULL    '\0'

void memset(void *buffer, uint8_t ch, uint32_t size);
#endif

