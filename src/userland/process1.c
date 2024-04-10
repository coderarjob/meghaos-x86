#include <kdebug.h>
#include <x86/cpu.h>
#include <types.h>
#include <process.h>

U32 syscall (U32 fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5);
void another_thread();

void userland_main()
{
    kbochs_breakpoint();
    char* console_text = "Console message from process";
    syscall (0, (PTR)console_text, 0, 0, 0, 0);

    U32 pid = syscall (1, (U32)&another_thread, 0,
                       PROCESS_FLAGS_THREAD | PROCESS_FLAGS_KERNEL_PROCESS, 0, 0);
    if (pid != 0) {
        syscall (2, pid, 0, 0, 0, 0);
    }

    while (1)
        ;
}

void another_thread()
{
    char* console_text = "Console message from thread";
    syscall (0, (PTR)console_text, 0, 0, 0, 0);

    U32 cr3 = 0xF00;
    x86_READ_REG (CR3, cr3);

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
