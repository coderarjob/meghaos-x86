#include <kdebug.h>
#include <x86/cpu.h>

static void foo();

void userland_main()
{
    kbochs_breakpoint();
    foo();

    // int cr3;
    // x86_READ_REG(CR3, cr3);

    while (1)
        ;
}

static void foo()
{
    kbochs_breakpoint();
    // int retval = 0;
    //__asm__ volatile("mov eax, 0xaaaa;"
    //                  "mov ebx, 0xbbbb;"
    //                  "mov ecx, 0xcccc;"
    //                  "mov edx, 0xdddd;"
    //                  "mov esi, 0xeeee;"
    //                  "mov edi, 0x1111;"
    //                  "INT 0x50":"=m"(retval));

    char* fmt           = "Ebx has value: 0x%x";
    volatile int number = 0xBB24;
    __asm__ volatile("int 0x50" : : "a"(0), "b"(fmt), "c"(&number) :);
}
