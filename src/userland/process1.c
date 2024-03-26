#include <kdebug.h>
void userland_main()
{
    kbochs_breakpoint();
    __asm__ volatile ("INT 0x40");
    while(1);
}
