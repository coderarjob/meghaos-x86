#include <kdebug.h>
#include <x86/cpu.h>
#include <types.h>

U32 syscall (U32 fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5);

void userland_main()
{
    kbochs_breakpoint();
    char* info_text = "Info from process 1";
    syscall (0, 0, (PTR)info_text, 0, 0, 0);

    char* error_text = "Error from process 1";
    syscall (0, 1, (PTR)error_text, 0, 0, 0);

    char* console_text = "Console message from process 1";
    syscall (1, (PTR)console_text, 0, 0, 0, 0);

    while (1)
        ;
}

U32 syscall (U32 fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5)
{
    U32 retval = 0;
    __asm__ volatile("int 0x50"
                     : "=a"(retval) // This is required. Otherwise compiler will not know that eax
                                    // will be changed after this instruction.
                     : "a"(fn), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5)
                     :);
    return retval;
}
