/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Debug console
*
* Uses the port 0xE9 hack to output characters to the linux Qemu console.
* ---------------------------------------------------------------------------
*
* Dated: 8th June 2021
*/
#include <kernel.h>

#if  defined(DEBUG)

static void s_e9puts(const char *string)
{
    char c;
    while((c = *string++))
        outb(0xE9, c);
}

void printk_debug_gs(const char *fmt, ...)
{
    char buffer[MAX_PRINTABLE_STRING_LENGTH];
    va_list l;

    va_start(l,fmt);
    vsnprintk(buffer, ARRAY_LENGTH(buffer), fmt, l);
    va_end(l);

#if (DEBUG_LEVEL & 1)
    // Print to E9 port 
    s_e9puts(buffer);
#endif

#if (DEBUG_LEVEL & 2)
    // Prints to the vga buffer
    int attr;
    kdisp_ioctl(DISP_GETATTR, &attr);

    kdisp_ioctl(DISP_SETATTR, disp_attr(BLACK,DARK_GRAY,0));
    printk(buffer);

    kdisp_ioctl(DISP_SETATTR, attr);
#endif
}

#endif // DEBUG
