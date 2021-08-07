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

typedef enum type_modifiers_tag {
    INT, LONG, LONG_LONG
} type_modifiers_t; 

typedef enum processing_states_tag{
    TYPE,CONVERTION,LITERAL,ERROR, EXIT
} processing_states_t; 


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

typedef struct pf_tag{
    struct kconsole_operations *cop;
    type_modifiers_t type_mod;
    const char *fmt;
    const char *p;
    va_list *list;
} pf_t;

static void pnum_base(struct kconsole_operations*, u64 num, u32 base);
static void pstring(struct kconsole_operations*, const char *s);
static u64 get_int_argument(va_list *list, type_modifiers_t type_modifier);

void printk(u8 type, const char *fmt, ...)
{

    va_list list;               
    va_start (list, fmt);
    vprintk(type, fmt, list);
    va_end(list);
}

static processing_states_t _convertion(pf_t *pf) 
{
    u64 d;
    char *s;
    
    const char c = *pf->fmt;

    switch(c){
        case 'x':
            d = get_int_argument(pf->list,pf->type_mod);
            pnum_base(pf->cop,d,16);
            break;
        case 'u':
            d = get_int_argument(pf->list,pf->type_mod);
            pnum_base(pf->cop,d,10);
            break;
        case 'o':
            d = get_int_argument(pf->list,pf->type_mod);
            pnum_base(pf->cop,d,8);
            break;
        case 'b':
            d = get_int_argument(pf->list,pf->type_mod);
            pnum_base(pf->cop,d,2);
            break;
        case 's':
            s = va_arg(*pf->list,char *);
            pstring(pf->cop,s);
            break;
        case '%':
            pf->cop->putc('%');
            break;
        default:
            return ERROR;
            break;
    } 

    pf->fmt++;
    return LITERAL;
}

static processing_states_t _type(pf_t *pf)
{
    const char c = *pf->fmt;
    if (c != 'l')
        return CONVERTION;

    if (pf->type_mod == LONG_LONG)
        return ERROR;

    pf->type_mod = (pf->type_mod == LONG)?LONG_LONG:LONG;
    pf->fmt++;
    return TYPE;
}

static processing_states_t _litetal(pf_t *pf)
{
    const char c = *pf->fmt;
    switch(c)
    {
        case '%':
            pf->p = pf->fmt++;
            pf->type_mod = INT;
            return TYPE;
            break;
        case '\0':
            return EXIT;
            break;
        default:
            pf->cop->putc(c);
            pf->fmt++;
            break;
    }
    return LITERAL;
}

/* Prints formatted on screen at the cursor location.*/
void vprintk(u8 type, const char *fmt, va_list list)
{
    // Print to debug cnsole only if DEBUG macro is set.
#ifndef DEBUG
    if (type == PK_DEBUG) return;
#endif

    processing_states_t state = LITERAL;
    pf_t pf_info = {
        .cop = &conop[type],
        .fmt = fmt,
        .type_mod = INT,
        .list = &list,
    };

    while(state != EXIT){
        switch(state) {
            case LITERAL:
                state = _litetal(&pf_info);
                break;
            case TYPE:
                state = _type(&pf_info);
                break;
            case CONVERTION:
                state = _convertion(&pf_info);
                break;
            case ERROR:
                while(pf_info.p <= pf_info.fmt)
                    pf_info.cop->putc(*pf_info.p++);

                if (*pf_info.fmt == '\0'){
                    state = EXIT;
                }
                else {
                    state = LITERAL;
                    pf_info.fmt++;
                }
                break;
            case EXIT:
                // Keeps the compiler happy.
                break;
        }
    }
}

static u64 get_int_argument(va_list *list, type_modifiers_t type_modifier)
{
    switch (type_modifier)
    {
        case LONG:
            return va_arg(*list,unsigned long int);
            break;
        case LONG_LONG:
            return va_arg(*list,unsigned long long int);
            break;
        default:
            return va_arg(*list,unsigned int);
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
