#include <types.h>
#include <cm.h>

void another_thread();

void proc_main()
{
    cm_putstr("\n  Process 0 - Running");
    cm_putstr("\n  Process 0 - Creating Thread 1");
    cm_thread_create(another_thread, false);

    for (int i = 0; i < 2; i++) {
        cm_putstr("\n  Process 0 - Yielding");
        cm_process_yield();
        cm_putstr("\n  Process 0 - Running");
    }

    cm_putstr("\n  Process 0 - Exiting");
    cm_process_kill(1);
    cm_putstr("\n   Process 0 - Not exited. Its is the only one.");
}

void another_thread()
{
    cm_putstr("\n  Thread 1 - Running");
    cm_putstr("\n  Thread 1 - Yielding");
    cm_process_yield();

    cm_putstr("\n  Thread 1 - Running");
    cm_putstr("\n  Thread 1 - Exiting");
    cm_process_kill(2);
}
