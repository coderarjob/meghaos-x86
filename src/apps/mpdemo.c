#include <types.h>
#include <syscall.h>
#include <cm.h>

#define MAX_VGA_COLUMNS             80U

typedef enum DisplayVgaColors
{
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    LIGHT_GRAY,
    DARK_GRAY,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    PINK,
    YELLOW,
    WHITE
} DisplayVgaColors;

static void s_printString (U32 row, U32 col, U32 bgcolor, U32 fgcolor, char* text);
static void s_progressbar (UINT iterPerStep, char* title, UINT row, UINT color);
static void thread0();
static void thread1();

static void s_printString (U32 row, U32 col, U32 bgcolor, U32 fgcolor, char* text)
{
    syscall (OSIF_SYSCALL_CONSOLE_SETCOLOR, bgcolor, fgcolor, 0, 0, 0);
    syscall (OSIF_SYSCALL_CONSOLE_SETCURSORPOS, row, col, 0, 0, 0);
    cm_putstr (text);
}

void s_progressbar (UINT delay_ms, char* title, UINT row, UINT color)
{
    s_printString (row, 0, BLACK, DARK_GRAY, title);

    UINT max = MAX_VGA_COLUMNS;

    for (UINT i = 0; i < max; i++) {
        s_printString (row + 2, i, color, DARK_GRAY, " ");
        cm_delay (delay_ms);
    }
}

void proc_main()
{
    cm_thread_create (thread0, false);
    cm_thread_create (&thread1, false);

    s_progressbar (200, "Process 0:\n", 26, RED);

    s_progressbar (20, "Process 0:\n", 26, RED);

    cm_process_kill(1);
    s_printString (37, 0, BLACK, WHITE,
                   "Cannot kill process 0. Make kernel thread slower for demo.");
}

void thread0()
{
    s_progressbar (10, "Thread 0:\n", 30, GREEN);
    cm_process_kill(2);
}

void thread1()
{
    s_progressbar (5, "Thread 1:\n", 34, YELLOW);
    cm_process_kill(3);
}
