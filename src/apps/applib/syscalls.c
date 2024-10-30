/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - User Library - System calls
 * -------------------------------------------------------------------------------------------------
 */

/* Note:
 * This library talks to the OS and needs to make sense of the result the OS provides. In order to
 * do these the library may require the configurations of the Kernel itself (for example to convert
 * tick count to micro seconds etc). Due to this tight coupling is between this library and the
 * kernel it makes sense to simply use the definations in kernel headers. However no public function
 * of this library should expose OS details, like data structures to application space. This library
 * (and its headers) whould act as the abstraction between the Kernel and the application programs.
 * This library should change with OS in order to shield the applications from the changes.
 */

#include <applib/app.h>
#include <config.h>
#include <kernel.h>
#include <process.h>

S32 syscall (OSIF_SYSCALLS fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5)
{
    S32 retval = 0;
    __asm__ volatile("int 0x50"
                     : "=a"(retval) // This is required. Otherwise compiler will not know that eax
                                    // will be changed after this instruction.
                     : "a"(fn), "b"(arg1), "c"(arg2), "d"(arg3), "S"(arg4), "D"(arg5)
                     :);
    return retval;
}

INT sys_process_create (void* startLocation, SIZE binaryLengthBytes, bool isKernelMode)
{
    KProcessFlags flags = PROCESS_FLAGS_NONE;
    if (isKernelMode) {
        flags |= PROCESS_FLAGS_KERNEL_PROCESS;
    }

    return syscall (OSIF_SYSCALL_CREATE_PROCESS, (U32)startLocation, binaryLengthBytes, (U32)flags, 0,
                    0);
}

INT sys_thread_create (void (*startLocation)(), bool isKernelMode)
{
    KProcessFlags flags = PROCESS_FLAGS_THREAD;
    if (isKernelMode) {
        flags |= PROCESS_FLAGS_KERNEL_PROCESS;
    }
    return syscall (OSIF_SYSCALL_CREATE_PROCESS, (U32)startLocation, 0, (U32)flags, 0, 0);
}

UINT os_tick_microseconds()
{
    UINT tick = sys_get_tickcount();
    return KERNEL_TICK_COUNT_TO_MICROSEC (tick);
}

// This is the entry point for all processes.
__attribute__ ((section (".entry.text")))
void proc_start()
{
    __asm__("jmp proc_main;");
    while (1)
        ;
}
