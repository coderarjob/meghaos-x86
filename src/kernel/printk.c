/*
* -----------------------------------------------------------------------------
* Megha Operating System V2 - Cross platform Kernel - Platform independent 
* printk type of functions.
*
* This is not and never going to be a full featured VGA text mode driver.
* Functions here will be limited to prints or printf like functions.
* NOTE: Actual VGA driver will be a server in User Mode.
* -----------------------------------------------------------------------------
*
* Dated: 5th September 2021
*/

#include <kernel.h>
#include <stdarg.h>
#include <stdbool.h>

enum IntTypes {
    INT, LONGINT, LONGLONGINT
};

static void s_prnstr(const char *str);
static void s_itoa(char **dest, size_t *size, u64 num, u32 base);
static bool s_convert(char **dest, size_t *size, enum IntTypes inttype, char c, 
                      va_list *l);
static enum IntTypes s_readtype(const char **fmt, char *c);
static u64 s_readint(enum IntTypes inttype, va_list *l);

int printk(const char *fmt, ...)
{
    va_list l;
    va_start(l, fmt);

    char buffer[MAX_PRINTABLE_STRING_LENGTH];
    int len = vsnprintk(buffer, ARRAY_LENGTH(buffer), fmt, l);

    va_end(l);

    s_prnstr(buffer);    
    return len;
}

int vsnprintk(char *dest, size_t size, const char *fmt, va_list l)
{
    size_t originalsize = size;

    char c;
    while((c = *fmt) && size > 1)
    {
        bool isliteral = true;
        const char *prevfmt = fmt;

        if (c == '%')
        {
            enum IntTypes inttype = s_readtype(&fmt, &c);
            isliteral = s_convert(&dest, &size, inttype, c, &l);
        }

        while (prevfmt <= fmt && size > 1 && isliteral)
        {
            *dest++ = *prevfmt++;
            size--;
        }

        fmt++;
    }

    *dest = '\0';
    size--;

    return originalsize - size;
}

static enum IntTypes s_readtype(const char **fmt, char *c)
{
    enum IntTypes inttype = INT;
    while ((*c = *++(*fmt)))
    {
        if (*c == 'l')
            inttype = (inttype == INT) ? LONGINT : LONGLONGINT;
        else
            break;
    }
    return inttype;
}
static u64 s_readint(enum IntTypes inttype, va_list *l)
{
    u64 value;
    switch(inttype)
    {
        case INT:
            value = va_arg(*l, u32);
            break;
        case LONGINT:
            value = va_arg(*l, u32);
            break;
        case LONGLONGINT:
            value = va_arg(*l, u64);
            break;
        default:
            value = 0xCAFEBABE189;
            break;
    }
    return value;
}
static bool s_convert(char **dest, size_t *size, enum IntTypes inttype, char c, 
                      va_list *l)
{
    u64 intvalue;
    char *stringvalue;
    bool isliteral = false;

    switch(c)
    {
        case 'u':
            intvalue = s_readint(inttype, l);
            s_itoa(dest, size, intvalue, 10);
            break;
        case 'x':
            intvalue = s_readint(inttype, l);
            s_itoa(dest, size, intvalue, 16);
            break;
        case 'b':
            intvalue = s_readint(inttype, l);
            s_itoa(dest, size, intvalue, 2);
            break;
        case 'o':
            intvalue = s_readint(inttype, l);
            s_itoa(dest, size, intvalue, 8);
            break;
        case 's':
            stringvalue = va_arg(*l, char *);
            while(*stringvalue && *size > 1)
            {
                *(*dest)++ = *stringvalue++;
                (*size)--;
            }
            break;
        case '%':
            *(*dest)++ = '%';
            (*size)--;
            break;
        default:
            isliteral = true;
            break;
    }

    return isliteral;
}
/* Displays a 64 bit number in various representation on the screen.  */
static void s_itoa(char **dest, size_t *size, u64 num, u32 base) 
{
    char *chars = "0123456789ABCDEF";

    // Holds space for the largest possible representation. 
    // Example: 65 characters in case of 64 bit int. (Base 2)
    char output[65] = {0};       
    int i = 0;
    do{
        output[i++] = chars[num % base];
        num /= base;
    }while(num > 0);

    // Display the characters in output in reverse order.
    while(i-- && *size > 1) 
    {
        *(*dest)++ = output[i];
        (*size)--;
    }
}

static void s_prnstr(const char *str)
{
    for(;*str;str++)
        kdisp_putc(*str);
}
