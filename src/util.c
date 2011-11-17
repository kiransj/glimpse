#include <util.h>
#include <display.h>
static void setToZero(void *buffer, uint32_t size)
{
    uint32_t tmp_size = size >> 2;
    uint32_t remaining_bytes = size & 0x03;
    if(tmp_size != 0)
    {
        uint32_t i = 0;
        uint32_t *buf = (uint32_t*)buffer;
        for(i = 0; i < tmp_size; i++)
        {
            buf[i] = 0;
        }
    }
    if(0 != remaining_bytes)
    {
        uint32_t i = tmp_size << 2;
        uint8_t *buf = (uint8_t*)buffer;
        for(i = tmp_size << 2; i < size; i++)
        {
            buf[i] = 0;
        }
    }
}

void memset(void *buffer, uint8_t ch, uint32_t size)
{
    if(0 == ch)
    {
        setToZero(buffer, size);

    }
    else
    {
        uint32_t i = 0;
        uint8_t *buf = (uint8_t*)buffer;
        for(i = 0; i < size; i++)
        {
            buf[i] = ch;
        }
    }
}

// Write a byte out to the specified port.
void outb(uint16_t port, uint8_t value)
{
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

uint16_t inw(uint16_t port)
{
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

uint32_t strlen(char *str)
{
    uint32_t i = 0;
    while(str[i++] != '\0');
    return i;
}

uint32_t strcpy(char *dest, char *src)
{
    uint32_t i = 0;
    while(src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return i;
}
