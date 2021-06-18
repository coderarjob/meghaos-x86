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

enum _type_modifiers {
    INT, LONG, LONG_LONG
}; 

enum _processing_states{
    FLAGS,WIDTH,PRECISION,TYPE,CONVERTION,LITERAL,ERROR,COMPLETED
}; 

// I could not simplly print to port E9 everytime printf is called. That way 
// the messages will always be printed on screen but only displayed to console 
// if DEBUG is set. Only way to disable printing to screen is to delete the 
// printk statements.  Because of the above problem, and to have better 
// control which message is printed where, I have two separate 'destinations' 
// one vga and another debug.  Note that the kdisp_* and kdebug_* functions 
// can have different implementations based on the target architecure or debug 
// environment. But the implementation of vprintk and printk should not change.
static struct kconsole_operations{
    void (*putc)(char c);
} conop[] = {
    { .putc = kdisp_putc },
    { .putc = kdebug_putc }
};

static void pnum_base(struct kconsole_operations*, u64 num, u32 base);
static void pstring(struct kconsole_operations*, const char *s);
static u64 _get_int_arg(va_list *list, enum _type_modifiers type_modifier);

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

static enum _processing_states 
_convertion_processing(struct kconsole_operations *cop, 
                              enum _type_modifiers type_mod,
                              const char *template, va_list *list)
{
    u64 d;
    char *s;

    switch(*template){
        case 'x':
            d = _get_int_arg(list,type_mod);
            pnum_base(cop,d,16);
            break;
        case 'd':
            d = _get_int_arg(list,type_mod);
            pnum_base(cop,d,10);
            break;
        case 'o':
            d = _get_int_arg(list,type_mod);
            pnum_base(cop,d,8);
            break;
        case 'b':
            d = _get_int_arg(list,type_mod);
            pnum_base(cop,d,2);
            break;
        case 's':
            s = va_arg(*list,char *);
            pstring(cop,s);
            break;
        case '%':
            cop->putc('%');
            break;
        default:
            return ERROR;
            break;
    } 

    return COMPLETED;
}

static enum _processing_states 
_type_processing(const char *template, enum _type_modifiers *tmod)
{
    switch(*template){
        case '%':
            break;
        case 'l':
            *tmod = (*tmod == LONG) ? LONG_LONG: LONG;
            break;
        default:
            return CONVERTION;
    }

    return TYPE;
}

/* Prints formatted on screen at the cursor location.*/
void vprintk(u8 type, const char *fmt, va_list list)
{
    enum _type_modifiers type_mod;
    enum _processing_states state;
    struct kconsole_operations *cop = &conop[type];

    for (const char *c = fmt ; *c; c++) {
        type_mod = INT;
        state = (*c == '%') ? TYPE : LITERAL;

        const char *cs = c;
        while(*cs && state != COMPLETED && state != LITERAL) {
            switch(state){
                case FLAGS:
                    break;
                case WIDTH:
                    break;
                case PRECISION:
                    break;
                case TYPE:
                    if ((state = _type_processing(cs, &type_mod)) == TYPE)
                        cs++;
                    break;
                case CONVERTION:
                    state = _convertion_processing(cop, type_mod, cs, &list);
                    if (state == ERROR)
                        state = LITERAL;
                    else
                        c = cs;
                    break;
                default:
                    break;
            }
        }

        if (state == LITERAL)
            cop->putc(*c);
    }
}

static u64 _get_int_arg(va_list *list, enum _type_modifiers type_modifier)
{
    switch (type_modifier)
    {
        case LONG:
            return va_arg(*list,long int);
            break;
        case LONG_LONG:
            return va_arg(*list,long long int);
            break;
        case INT:
            return va_arg(*list,int);
            break;
    }
}

/* Displays a 64 bit number in various representation on the screen.  */
static void pnum_base(struct kconsole_operations *cop, u64 num, u32 base) 
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
    while(i--)
        cop->putc(output[i]);
}

static void pstring(struct kconsole_operations *cop, const char *s)
{
    for(;*s;s++)
        cop->putc(*s);
}
