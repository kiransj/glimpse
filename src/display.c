#include <util.h>

#include <display.h>
#define NUM_OF_ROWS         25
#define NUM_OF_COLUMNS      80


static uint8_t * const videoram = (uint8_t *) 0xb8000;
static uint32_t row = 0, col = 0;

static void scroll(void);
static uint32_t write_string(char *string);
static void print_uint32(uint32_t tmp, uint32_t base);


void clear_screen(void)
{
    memset(videoram, 0, NUM_OF_ROWS*NUM_OF_COLUMNS*2);
    return ;
}

void put_char(char ch)
{
    if(row == NUM_OF_ROWS)
    {
        scroll();
        row--;
    }
    if('\n' == ch)
    {
        row++;
        col = 0;
    }
    else if('\r' == ch)
    {
        col = 0;
    }
    else
    {
        uint32_t current_position = (row * NUM_OF_COLUMNS + col)*2;
        videoram[current_position] = ch;
        videoram[current_position+1] = 0x0F;
        col++;
        if(col == NUM_OF_COLUMNS)
        {
            row++;
            col = 0;
        }
    }

}

void scroll(void)
{
    uint32_t start_offset = NUM_OF_COLUMNS * 2;
    uint32_t screen_size = (NUM_OF_ROWS-1) * NUM_OF_COLUMNS * 2;
    uint32_t i;

    for(i = 0; i < screen_size; i++)
    {
        videoram[i] = videoram[start_offset + i];
    }

    start_offset = NUM_OF_COLUMNS * 2 * (NUM_OF_ROWS-1);
    screen_size = (NUM_OF_ROWS) * NUM_OF_COLUMNS * 2;

    for(i = start_offset; i < screen_size; i++)
    {
        videoram[i] = 0;
    }
    return;
}

uint32_t write_string(char *string)
{
    uint32_t i = 0;
    while(NULL != string[i])
    {
        put_char(string[i]);
        i++;
    }
    return i;
}


void print_uint32(uint32_t num, uint32_t base)
{
    uint32_t tmp = num, i = 0;
    char number[32] = { NULL };
    do
    {
        uint32_t rem = tmp % base;

        if(rem < 10)
            number[i++] = '0' + rem;
        else if(rem >= 10)
            number[i++] = 'A' + (rem - 10);

        if(16 ==base)
            tmp = tmp >> 4;
        else
            tmp = tmp/base;
    }
    while(0 != tmp);

    i--;
    while(i > 0)
    {
        put_char(number[i]);
        i--;
    }
    put_char(number[0]);
    return;
}

void print_int(int num)
{
    int tmp = num;

    if(tmp < 0)
    {
        put_char('-');
        tmp = -tmp;
    }
    print_uint32((uint32_t)tmp, 10);
    return;
}
uint32_t printf(char *format, ...)
{
    uint32_t i = 0;
    va_list ap;
    va_start(ap, format);

    while(NULL != format[i])
    {
        if(format[i] == '%')
        {
            i++;
            switch(format[i])
            {
                case 'd':
                    {
                        int tmp;
                        tmp = va_arg(ap, int);
                        print_int(tmp);
                        break;
                    }
                case 'u':
                    {
                        uint32_t tmp;
                        tmp = va_arg(ap, uint32_t);
                        print_uint32(tmp, 10);
                        break;
                    }
                case 'x':
                    {
                        uint32_t tmp;
                        tmp = va_arg(ap, uint32_t);
                        put_char('0');
                        put_char('x');
                        print_uint32(tmp, 16);
                        break;
                    }
                case 's':
                    {
                        char *tmp;
                        tmp = va_arg(ap, char*);
                        write_string(tmp);
                        break;
                    }
                case 'c':
                    {
                        char ch;
                        int tmp = va_arg(ap, int);
                        ch = (int)tmp;
                        put_char(ch);
                        break;
                    }
                case NULL:
                default:
                    goto END;
            }
        }
        else
        {
            put_char(format[i]);
        }
        i++;
    }
END:
    va_end(ap);
    return 0;
}
