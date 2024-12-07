#include <stdbool.h>
#include <types.h>
#include <syscall.h>
#include <cm.h>
#include <debug.h>

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

bool all_child_exited = false;

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

static void s_progressbar (UINT delay_ms, char* title, UINT row, UINT color)
{
    s_printString (row, 0, BLACK, DARK_GRAY, title);

    UINT max = MAX_VGA_COLUMNS;

    for (UINT i = 0; i < max; i++) {
        s_printString (row + 2, i, color, DARK_GRAY, " ");
        cm_delay (delay_ms);
    }
}

static void count_child_exits (OSIF_ProcessEvent const* const e)
{
    (void)e;
    static int count = 0;

    CM_DBG_INFO ("Child process exited.");
    if (++count == 2) {
        CM_DBG_INFO ("Every child process exited.");
        all_child_exited = true;
    }
}

static void wait_for_all_child_exit()
{
    while (!all_child_exited) {
        cm_process_handle_events();
    }
}

void proc_main()
{
    cm_thread_create (thread0, false);
    cm_thread_create (thread1, false);

    cm_process_register_event_handler (OSIF_PROCESS_EVENT_PROCCESS_CHILD_KILLED,
                                       count_child_exits);

    s_progressbar (5, "Process 0:\n", 26, RED);

    wait_for_all_child_exit();

    cm_process_kill(1);
}

void thread0()
{
    s_progressbar (10, "Thread 0:\n", 30, GREEN);
    cm_process_kill(2);
}

void thread1()
{
    s_progressbar (2, "Thread 1:\n", 34, YELLOW);
    cm_process_kill(3);
}
