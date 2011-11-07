#ifndef _DISPLAY_H
#define _DISPLAY_H

#include <util.h>
#include <stdarg.h>

void clear_screen(void);

void put_char(char ch);
uint32_t printf(char *format, ...);
#endif
