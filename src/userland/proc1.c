#include <types.h>
#include <moslib/ulib.h>

void another_thread();

void proc_main()
{
    sys_putstr("\n  Process 0 - Running");
    sys_putstr("\n  Process 0 - Creating Thread 1");
    sys_thread_create(another_thread, false);

    for (int i = 0; i < 3; i++) {
        sys_putstr("\n  Process 0 - Yielding");
        sys_yield();
        sys_putstr("\n  Process 0 - Running");
    }

    sys_putstr("\n  Process 0 - Exiting");
    sys_process_kill();
    sys_putstr("\n   Process 0 - Not exited. Its is the only one.");
}

void another_thread()
{
    sys_putstr("\n  Thread 1 - Running");
    sys_putstr("\n  Thread 1 - Yielding");
    sys_yield();

    sys_putstr("\n  Thread 1 - Running");
    sys_putstr("\n  Thread 1 - Exiting");
    sys_process_kill();
}
