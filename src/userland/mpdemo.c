#include <kdebug.h>
#include <x86/cpu.h>
#include <types.h>
#include <process.h>
#include <x86/vgatext.h>
#include <moslimits.h>

U32 syscall (U32 fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5);
void thread0();
void thread1();

// This is the entry point for process.
__asm__("jmp proc_main;");

void proc_main()
{
    kbochs_breakpoint();

    syscall (1, (U32)&thread0, 0, PROCESS_FLAGS_THREAD, 0, 0);
    syscall (1, (U32)&thread1, 0, PROCESS_FLAGS_THREAD, 0, 0);

    syscall (4, BLACK, DARK_GRAY, 0, 0, 0);
    syscall (5, 30, 0, 0, 0, 0);
    syscall (0, (PTR) "Process 0:\n", 0, 0, 0, 0);

    UINT column = 0;
    UINT max    = 95000;
    UINT step   = max / MAX_VGA_COLUMNS;

    for (UINT i = 0; i < max; i++) {
        syscall (5, 31, column, 0, 0, 0);
        syscall (4, RED, DARK_GRAY, 0, 0, 0);
        if (i % step == 0) {
            column = (column + 1) % MAX_VGA_COLUMNS;
        }
        syscall (0, (PTR) " ", 0, 0, 0, 0);
        syscall (2, 0, 0, 0, 0, 0);
    }

    syscall (3, 0, 0, 0, 0, 0);
    while (1)
        ;
}

void thread0()
{
    syscall (4, BLACK, DARK_GRAY, 0, 0, 0);
    syscall (5, 33, 0, 0, 0, 0);
    syscall (0, (PTR) "Thread 0:\n", 0, 0, 0, 0);

    UINT column = 0;
    UINT max    = 94000;
    UINT step   = max / MAX_VGA_COLUMNS;

    for (UINT i = 0; i < max; i++) {
        syscall (5, 34, column, 0, 0, 0);
        syscall (4, GREEN, DARK_GRAY, 0, 0, 0);
        if (i % step == 0) {
            column = (column + 1) % MAX_VGA_COLUMNS;
        }
        syscall (0, (PTR) " ", 0, 0, 0, 0);
        syscall (2, 0, 0, 0, 0, 0);
    }

    syscall (3, 0, 0, 0, 0, 0);
}

void thread1()
{
    syscall (4, BLACK, DARK_GRAY, 0, 0, 0);
    syscall (5, 36, 0, 0, 0, 0);
    syscall (0, (PTR) "Thread 1:\n", 0, 0, 0, 0);

    UINT column = 0;
    UINT max    = 94000;
    UINT step   = max / MAX_VGA_COLUMNS;

    for (UINT i = 0; i < max; i++) {
        syscall (5, 37, column, 0, 0, 0);
        syscall (4, YELLOW, DARK_GRAY, 0, 0, 0);
        if (i % step == 0) {
            column = (column + 1) % MAX_VGA_COLUMNS;
        }
        syscall (0, (PTR) " ", 0, 0, 0, 0);
        syscall (2, 0, 0, 0, 0, 0);
    }

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
