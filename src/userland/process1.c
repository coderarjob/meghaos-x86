#include <kdebug.h>

static void foo();

void userland_main()
{
    kbochs_breakpoint();
    foo();
    while(1);
}

static void foo()
{
    __asm__ volatile ("INT 0x40");
}
