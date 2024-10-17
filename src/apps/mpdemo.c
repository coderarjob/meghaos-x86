#include <types.h>
#include <x86/vgatext.h>
#include <moslimits.h>
#include <applib/app.h>

static void s_printString (U32 row, U32 col, U32 bgcolor, U32 fgcolor, char* text);
static void s_progressbar (UINT iterPerStep, char* title, UINT row, UINT color);
static void thread0();
static void thread1();

static void s_printString (U32 row, U32 col, U32 bgcolor, U32 fgcolor, char* text)
{
    syscall (SYSCALL_CONSOLE_SETCOLOR, bgcolor, fgcolor, 0, 0, 0);
    syscall (SYSCALL_CONSOLE_SETCURSORPOS, row, col, 0, 0, 0);
    sys_putstr (text);
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
        sys_yield();
    }

    sys_process_kill();
}

void proc_main()
{
    IProcessEvent e;
    sys_process_pop_event (2, &e);
    if (e.event == 0x10 && e.data == 0x20) {
        s_printString (40, 0, BLACK, DARK_GRAY, "Event received.\n");
    }

    sys_thread_create (thread0, false);
    sys_thread_create (&thread1, false);

    s_printString (25, 0, BLACK, DARK_GRAY, "Process 0:\n");

    UINT iterPerStep = 11;

    s_progressbar (iterPerStep, "Process 0:\n", 25, RED);

    sys_process_kill();
    s_printString (37, 0, BLACK, WHITE,
                   "Cannot kill process 0. Make kernel thread slower for demo.");
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
