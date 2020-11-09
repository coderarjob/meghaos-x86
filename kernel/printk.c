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
* Dated: 24th October 2020
*/

#include <kernel.h>
#include <stdarg.h>

static void phex(u32 num);
static void pstring(const char *s);

void printk(u8 type, const char *fmt, ...)
{
    va_list list;               
    va_start (list, fmt);
    
    switch(type)
    {
        case PK_ONSCREEN:
            vprintk(fmt, list);
            break;
        default:
            kassert(0,"Invalid type in printk");
            break;
    }

    va_end(list);
}

/* Prints formatted on screen at the cursor location.*/
void vprintk(const char *fmt, va_list list)
{
    u32 d;
    const char *s;

    for (const char *c = fmt;*c ; c++)
    {
        if (*c == '%') {
            switch (*++c) {
                case '%':
                    kdisp_putc('%');
                    break;
                case 'x':
                    d = va_arg(list,u32);
                    phex(d);
                    break;
                case 's':
                    s = va_arg(list,char *);
                    pstring(s);
                    break;
                default:
                    // No need to show any error.
                    // No need to panic either.
                    break;
            }
            continue;
        }

        kdisp_putc(*c);
    }
}

/* Displays a 32 bit number in Hexadecimal representation on the screen.  */
static void phex(u32 num)
{
    char *hexchars = "0123456789ABCDEF";
    char output[9] = {0};

    for(int i = 7; i >= 0; i--, num >>= 4)
        output[i] = hexchars[num & 0xF];

    pstring(output);
}

static void pstring(const char *s)
{
    for(;*s;s++)
        kdisp_putc(*s);
}
