#include <types.h>
#include <applib/syscall.h>

void another_thread();

void proc_main()
{
    os_putstr("\n  Process 0 - Running");
    os_putstr("\n  Process 0 - Creating Thread 1");
    os_thread_create(another_thread, false);

    for (int i = 0; i < 3; i++) {
        os_putstr("\n  Process 0 - Yielding");
        os_yield();
        os_putstr("\n  Process 0 - Running");
    }

    os_putstr("\n  Process 0 - Exiting");
    os_process_kill();
    os_putstr("\n   Process 0 - Not exited. Its is the only one.");
}

void another_thread()
{
    os_putstr("\n  Thread 1 - Running");
    os_putstr("\n  Thread 1 - Yielding");
    os_yield();

    os_putstr("\n  Thread 1 - Running");
    os_putstr("\n  Thread 1 - Exiting");
    os_process_kill();
}
