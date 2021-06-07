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

// I could not simplly print to port E9 everytime printf is called. That way the messages will always be printed
// on screen but only displayed to console if DEBUG is set. Only way to disable printing to screen is to delete
// the printk statements.
// Because of the above problem, and to have better control which message is printed where, I have two separate
// 'destinations' one vga and another debug.
// Note that the kdisp_* and kdebug_* functions can have different implementations based on the target architecure
// or debug environment. But the implementation of vprintk and printk should not change.
struct kconsole_operations{
    void (*ioctl)(u8 request, ...);
    void (*init)();
    void (*putc)(char c);
};

struct kconsole_operations conop[] = {
    {
        .ioctl = kdisp_ioctl,
        .init  = kdisp_init,
        .putc = kdisp_putc
    },
    {
        .putc = kdebug_putc
    },
};

static void pnum_base(struct kconsole_operations*, u32 num, u32 base);
static void pstring(struct kconsole_operations*, const char *s);

void printk(u8 type, const char *fmt, ...)
{
    // Print to debug cnsole only if DEBUG macro is set.
    #ifndef DEBUG
        if (type == PK_DEBUG) return;
    #endif

    va_list list;               
    va_start (list, fmt);
    vprintk(type, fmt, list);
    va_end(list);
}

/* Prints formatted on screen at the cursor location.*/
void vprintk(u8 type, const char *fmt, va_list list)
{
    struct kconsole_operations *cop = &conop[type];

    int d;                   // Compiler always infers numeric literial as int,
                             // until 'L', 'LL', etc override is provided.
    const char *s;
    for (const char *c = fmt;*c ; c++)
    {
        if (*c == '%') {
            switch (*++c) {
                case '%':
                    cop->putc('%');
                    break;
                case 'x':
                    d = va_arg(list,int);
                    pnum_base(cop,d,16);
                    break;
                case 'd':
                    d = va_arg(list,int);
                    pnum_base(cop,d,10);
                    break;
                case 'o':
                    d = va_arg(list,int);
                    pnum_base(cop,d,8);
                    break;
                case 'b':
                    d = va_arg(list,int);
                    pnum_base(cop,d,2);
                    break;
                case 's':
                    s = va_arg(list,char *);
                    pstring(cop,s);
                    break;
                default:
                    // No need to show any error.
                    // No need to panic either.
                    break;
            }
            continue;
        }

        cop->putc(*c);
    }
}

/* Displays a 32 bit number in various representation on the screen.  */
static void pnum_base(struct kconsole_operations *cop, 
                      u32 num, u32 base)
{
    char *chars = "0123456789ABCDEF";
    
    // Holds space for the largest possible representation. 
    // Example: 33 characters in case of 32 bit int. (Base 2)
    char output[33] = {0};       
    int i = 0;
    do{
        output[i++] = chars[num % base];
        num /= base;
    }while(num > 0);

    // Display the characters in output in reverse order.
    while(i--)
        cop->putc(output[i]);
}

static void pstring(struct kconsole_operations *cop, const char *s)
{
    for(;*s;s++)
        cop->putc(*s);
}
