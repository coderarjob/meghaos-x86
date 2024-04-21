#include <kdebug.h>
#include <x86/cpu.h>
#include <types.h>
#include <process.h>

U32 syscall (U32 fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5);
void another_thread();

// This is the entry point for process.
__asm__("jmp proc_main;");

void proc_main()
{
    kbochs_breakpoint();

    syscall (1, (U32)&another_thread, 0, PROCESS_FLAGS_THREAD, 0, 0);

    for (int i = 0; i < 5; i++) {
        syscall (0, (U32) "From process", 0, 0, 0, 0);
        syscall (2, 0, 0, 0, 0, 0);
    }
    syscall (0, (U32) "Killing process", 0, 0, 0, 0);
    syscall (3, 0, 0, 0, 0, 0);
    while (1)
        ;
}

void another_thread()
{
    syscall (0, (PTR) "From process thread", 0, 0, 0, 0);
    syscall (2, 0, 0, 0, 0, 0);

    syscall (0, (PTR) "Killing process thread", 0, 0, 0, 0);
    syscall (3, 0, 0, 0, 0, 0);
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
