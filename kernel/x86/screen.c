/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Basic VGA Text driver.
*
* This is not and never going to be a full featured VGA text mode driver.
* Functions here will be limited to prints or printf like functions.
* NOTE: Actual VGA driver will be a server in User Mode.
* ---------------------------------------------------------------------------
*
* Dated: 6th October 2020
*/

#include <kernel.h>
#include <stdarg.h>

static u8 *vgabuff = (u8 *)0xb8000;
static int foo = 1043;

static void __phex(u32 num);
static void __kvprintf(const char *fmt, va_list list);

__attribute__((format (printf,1,2)))
void kprintf(const char *fmt, ...)
{
    va_list list;               
    va_start (list, fmt);
    kbochs_breakpoint();
    foo = va_arg(list,u32);
    kbochs_breakpoint();

    //__kvprintf(fmt, list);
    va_end(list);
}

static void __kvprintf(const char *fmt, va_list list)
{
    u32 d;
    const char *s;

    for (const char *c = fmt;*c ; c++)
    {
        if (*c == '%') {
            switch (*++c) {
                case '%':
                    kputc('%', VGA_TEXT_GREY);
                    break;
                case 'x':
                    d = va_arg(list,u32);
                    __phex(d);
                    break;
                case 's':
                    s = va_arg(list,char *);
                    kputs(s,VGA_TEXT_GREY);
                    break;
            }
            continue;
        }
        kputc(*c, VGA_TEXT_GREY);
    }
}

/*
 * Displays a 32 bit number in Hexadecimal representation on the screen.
 * */
static void __phex(u32 num)
{
    char *hexchars = "0123456789ABCDEF";
    char output[9] = {0};

    for(int i = 7; i >= 0; i--, num >>= 4)
        output[i] = hexchars[num & 0xF];

    kputs(output, VGA_TEXT_WHITE);
}

/*
 * Prints an ASCII character on the VGA text mode frame buffer
 * and increments the pointer to it.
 * */
void kputc(char c, u8 attribute)
{
    *vgabuff++ = c;
    *(vgabuff++) = attribute;
}

/*
 * Prints a ASCIZ string on the VGA text mode frame buffer
 * and increments the pointer to it.
 * */
void kputs(char *s, u8 attribute)
{
    for (;*s;s++)
        kputc(*s,attribute);
}

