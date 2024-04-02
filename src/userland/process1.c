#include <kdebug.h>
#include <x86/cpu.h>

static void foo();

void userland_main()
{
    kbochs_breakpoint();
    foo();

    int cr3;
    x86_READ_REG(CR3, cr3);

    while(1);
}

static void foo()
{
    kbochs_breakpoint();
    __asm__ volatile ("INT 0x40");
}
