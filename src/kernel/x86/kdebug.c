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
void kdebug_putc(char c)
{
    outb(0xE9, c);
}