/*
* -----------------------------------------------------------------------------
* Megha Operating System V2 - Cross platform Kernel - Platform independent 
* kearly_printf type of functions.
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

typedef enum IntTypes 
{
    INTEGER, LONGINT, LONGLONGINT
} IntTypes;

static void s_prnstr (const CHAR *str);
static void s_itoa (CHAR **dest, size_t *size, U64 num, U32 base);
static bool s_convert (CHAR **dest, size_t *size, IntTypes inttype, CHAR c, 
                      va_list *l);
static IntTypes s_readtype (const CHAR **fmt, CHAR *c);
static U64 s_readint (IntTypes inttype, va_list *l);

INT kearly_printf (const CHAR *fmt, ...)
{
    va_list l;
    va_start (l, fmt);

    CHAR buffer[MAX_PRINTABLE_STRING_LENGTH];
    INT len = kearly_vsnprintf (buffer, ARRAY_LENGTH (buffer), fmt, l);

    va_end (l);

    s_prnstr (buffer);    
    return len;
}

INT kearly_vsnprintf (CHAR *dest, size_t size, const CHAR *fmt, va_list l)
{
    size_t originalsize = size;

    CHAR c;
    while ((c = *fmt) && size > 1)
    {
        bool isliteral = true;
        const CHAR *prevfmt = fmt;

        if (c == '%')
        {
            IntTypes inttype = s_readtype (&fmt, &c);
            isliteral = s_convert (&dest, &size, inttype, c, &l);
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

    return (INT)(originalsize - size);
}

static IntTypes s_readtype (const CHAR **fmt, CHAR *c)
{
    IntTypes inttype = INTEGER;
    while ((*c = *++(*fmt)))
    {
        if (*c == 'l')
            inttype = (inttype == INTEGER) ? LONGINT : LONGLONGINT;
        else
            break;
    }
    return inttype;
}
static U64 s_readint (IntTypes inttype, va_list *l)
{
    U64 value;
    switch (inttype)
    {
        case INTEGER:
            value = va_arg (*l, U32);
            break;
        case LONGINT:
            value = va_arg (*l, U32);
            break;
        case LONGLONGINT:
            value = va_arg (*l, U64);
            break;
        default:
            value = 0xCAFEBABE189;
            break;
    }
    return value;
}
static bool s_convert (CHAR **dest, size_t *size, IntTypes inttype, CHAR c, 
                      va_list *l)
{
    U64 intvalue;
    CHAR *stringvalue;
    bool isliteral = false;

    switch (c)
    {
        case 'u':
            intvalue = s_readint (inttype, l);
            s_itoa (dest, size, intvalue, 10);
            break;
        case 'x':
            intvalue = s_readint (inttype, l);
            s_itoa (dest, size, intvalue, 16);
            break;
        case 'b':
            intvalue = s_readint (inttype, l);
            s_itoa (dest, size, intvalue, 2);
            break;
        case 'o':
            intvalue = s_readint (inttype, l);
            s_itoa (dest, size, intvalue, 8);
            break;
        case 's':
            stringvalue = va_arg (*l, CHAR *);
            while (*stringvalue && *size > 1)
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
static void s_itoa (CHAR **dest, size_t *size, U64 num, U32 base) 
{
    CHAR *chars = "0123456789ABCDEF";

    // Holds space for the largest possible representation. 
    // Example: 65 characters in case of 64 bit INT. (Base 2)
    CHAR output[65] = {0};       
    INT i = 0;
    do{
        output[i++] = chars[num % base];
        num /= base;
    }while (num > 0);

    // Display the characters in output in reverse order.
    while (i-- && *size > 1) 
    {
        *(*dest)++ = output[i];
        (*size)--;
    }
}

static void s_prnstr (const CHAR *str)
{
    for (;*str;str++)
        kdisp_putc (*str);
}
