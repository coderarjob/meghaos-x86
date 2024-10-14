#include <kdebug.h>
#include <x86/cpu.h>
#include <types.h>
#include <process.h>
#include <x86/vgatext.h>
#include <moslimits.h>

U32 syscall (U32 fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5);
static void s_printString (U32 row, U32 col, U32 bgcolor, U32 fgcolor, char* text);
void s_progressbar (UINT iterPerStep, char* title, UINT row, UINT color);
void thread0();
void thread1();

// This is the entry point for process.
__asm__("jmp proc_main;");

static void s_printString (U32 row, U32 col, U32 bgcolor, U32 fgcolor, char* text)
{
    syscall (4, bgcolor, fgcolor, 0, 0, 0);
    syscall (5, row, col, 0, 0, 0);
    syscall (0, (PTR)text, 0, 0, 0, 0);
}

void s_progressbar (UINT iterPerStep, char* title, UINT row, UINT color)
{
    s_printString (row, 0, BLACK, DARK_GRAY, title);

    UINT column = 0;
    UINT max    = iterPerStep * MAX_VGA_COLUMNS;

    for (UINT i = 0; i < max; i++) {
        s_printString (row + 2, column, color, DARK_GRAY, " ");
        if (i % iterPerStep == 0) {
            column = (column + 1) % MAX_VGA_COLUMNS;
        }
        syscall (2, 0, 0, 0, 0, 0);
    }

    syscall (3, 0, 0, 0, 0, 0);
}

void proc_main()
{
    kbochs_breakpoint();

    syscall (1, (U32)&thread0, 0, PROCESS_FLAGS_THREAD, 0, 0);
    syscall (1, (U32)&thread1, 0, PROCESS_FLAGS_THREAD, 0, 0);

    s_printString (25, 0, BLACK, DARK_GRAY, "Process 0:\n");

    UINT iterPerStep = 11;

    s_progressbar (iterPerStep, "Process 0:\n", 25, RED);

    syscall (3, 0, 0, 0, 0, 0);
    s_printString (37, 0, BLACK, WHITE,
                   "Cannot kill process 0. Make kernel thread slower for demo.");
    while (1)
        ;
}

void thread0()
{
    UINT iterPerStep = 10;

    s_progressbar (iterPerStep, "Thread 0:\n", 29, GREEN);
}

void thread1()
{
    UINT iterPerStep = 7;

    s_progressbar (iterPerStep, "Thread 1:\n", 33, YELLOW);
}
