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

    syscall (0, (U32) "\n  Process 0 - Running", 0, 0, 0, 0);
    syscall (0, (U32) "\n  Process 0 - Creating Thread 1", 0, 0, 0, 0);
    syscall (1, (U32)&another_thread, 0, PROCESS_FLAGS_THREAD, 0, 0);

    for (int i = 0; i < 3; i++) {
        syscall (0, (U32) "\n  Process 0 - Yielding", 0, 0, 0, 0);
        syscall (2, 0, 0, 0, 0, 0);
        syscall (0, (U32) "\n  Process 0 - Running", 0, 0, 0, 0);
    }

    syscall (0, (U32) "\n   Process 0 - Exiting", 0, 0, 0, 0);
    syscall (3, 0, 0, 0, 0, 0);
    syscall (0, (U32) "\n   Process 0 - Not exited. Its is the only one.", 0, 0, 0, 0);
    while (1)
        ;
}

void another_thread()
{
    syscall (0, (U32) "\n    Thread 1 - Running", 0, 0, 0, 0);
    syscall (0, (U32) "\n    Thread 1 - Yielding", 0, 0, 0, 0);
    syscall (2, 0, 0, 0, 0, 0);

    syscall (0, (U32) "\n    Thread 1 - Running", 0, 0, 0, 0);
    syscall (0, (PTR) "\n    Thread 1 - Exiting", 0, 0, 0, 0);
    syscall (3, 0, 0, 0, 0, 0);
}
