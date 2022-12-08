/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - Cross platform Kernel - Platform independent 
* kearly_printf type of functions.
*
* This is not and never going to be a full featured VGA text mode driver. Functions here will be
* limited to prints or printf like functions.
* --------------------------------------------------------------------------------------------------
*
* Dated: 5th September 2021
*/

#include <kernel.h>
#include <stdarg.h>
#include <stdbool.h>

typedef enum IntTypes 
{
    INTEGER, LONGINT, LONGLONGINT, ADDRESS
} IntTypes;

static void s_prnstr (const CHAR *str);
static void s_itoa (CHAR **dest, S64 *size, U64 num, U32 base);
static bool s_convert (CHAR **dest, S64 *size, IntTypes inttype, CHAR c, va_list *l);
static IntTypes s_readtype (const CHAR **fmt, CHAR *c);
static U64 s_readint (IntTypes inttype, va_list *l);

/***************************************************************************************************
 * Prints on screen the arguments in the format specified.
 *
 * @param fmt   Pointer to a string, with or without conversion attributes for the arguments.
 *              Conversion attributes are given in the following format:
 *                  %[ type modifiers : l|ll|p ] [ conversions: u|x|o|s|%  ]
 *
 *              For format details, see description for `kearly_vsnprintf`.
 * @param ...   Arguments for each conversion specification.
 * @return      For description for `kearly_vsnprintf`.
 **************************************************************************************************/
INT kearly_printf (const CHAR *fmt, ...)
{
    if (kdisp_isInitialized() == false)
        return 0;

    va_list l;
    va_start (l, fmt);

    CHAR buffer[MAX_PRINTABLE_STRING_LENGTH];
    INT len = kearly_vsnprintf (buffer, ARRAY_LENGTH (buffer), fmt, l);

    va_end (l);

    s_prnstr (buffer);    
    return len;
}

/***************************************************************************************************
 * Writes to a string pointer the arguments in the format specified.
 *
 * @param fmt   Pointer to a string, with or without conversion attributes for the arguments.
 *              Conversion attributes are given in the following format:
 *                  %[ type modifiers : l|ll|p ] [ conversions: u|x|o|s|%  ]
 *
 *              For format details, see description for `kearly_vsnprintf`.
 * @param ...   Arguments for each conversion specification.
 * @return      For description for `kearly_vsnprintf`.
 **************************************************************************************************/
INT kearly_snprintf (CHAR *dest, size_t size, const CHAR *fmt, ...)
{
    va_list l;
    va_start (l, fmt);

    INT len = kearly_vsnprintf (dest, size, fmt, l);
    va_end (l);

    return len;
}

/***************************************************************************************************
 * Writes to a string pointer (dest) the arguments in the format specified.
 *
 * @param dest  Pointer to a string, where the output will be written.
 * @param sz    Size of the string in bytes.
 * @param fmt   Pointer to a string, with or without conversion attributes for the arguments.
 *              Conversion attributes are given in the following format:
 *                  %[ type modifiers : l|ll|p ] [ conversions: u|x|o|s|%  ]
 *
 *              Type Modifiers   Function
 *              ---------------  -----------------------------------------------------------
 *              %l              Treats argument as a LONG integer.
 *              %llu            Treats argument as a LONG LONG integer.
 *              %p              Treats argument as a USYSINT (system address or large integer).
 *
 *              Conversions      Function
 *              ---------------  -----------------------------------------------------------
 *              %s               Treats argument a pointer to a char *.
 *              %u               Prints argument in base 10. Default type is UINT.
 *              %o               Prints argument in base 8. Default type is UINT.
 *              %x               Prints argument in base 16. Default type is UINT.
 *              %%               Prints a %.
 *
 * @param l     Arguments for each conversion specification.
 * @return      Upon successful return, these functions return the number of characters printed
 *              (excluding the null byte used to end output to strings).
 * @return      If the output was truncated due to the size (sz) limit, then the return value is
 *              the number of characters (excluding the terminating null byte) which would have been
 *              written  to  the final string if enough space had been available.
 *              A return value of size or more means that the output was truncated
 **************************************************************************************************/
INT kearly_vsnprintf (CHAR *dest, size_t size, const CHAR *fmt, va_list l)
{
    size_t originalsize = size;
    S64 size_left = size;

    CHAR c;
    while ((c = *fmt))
    {
        bool isliteral = true;
        const CHAR *prevfmt = fmt;

        if (c == '%')
        {
            IntTypes inttype = s_readtype (&fmt, &c);
            isliteral = s_convert (&dest, &size_left, inttype, c, &l);
        }

        while (prevfmt <= fmt && isliteral)
        {
            if (size_left > 1) *dest++ = *prevfmt;
            prevfmt++;
            size_left--;
        }

        fmt++;
    }

    *dest = '\0';
    return (INT)(originalsize - size_left);
}

static IntTypes s_readtype (const CHAR **fmt, CHAR *c)
{
    IntTypes inttype = INTEGER;
    while ((*c = *++(*fmt)))
    {
        if (*c == 'l')
            inttype = (inttype == INTEGER) ? LONGINT : LONGLONGINT;
        else if (*c == 'p')
            inttype = ADDRESS;
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
        case ADDRESS:
            value = va_arg (*l, USYSINT);
            break;
        default:
            value = 0xCAFEBABE189;
            break;
    }
    return value;
}
static bool s_convert (CHAR **dest, S64 *size, IntTypes inttype, CHAR c, va_list *l)
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
            while (*stringvalue)
            {
                if (*size > 1) *(*dest)++ = *stringvalue;
                stringvalue++;
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
static void s_itoa (CHAR **dest, S64 *size, U64 num, U32 base)
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
    while (i--)
    {
        if (*size > 1) *(*dest)++ = output[i];
        (*size)--;
    }
}

static void s_prnstr (const CHAR *str)
{
    for (;*str;str++)
        kdisp_putc (*str);
}
