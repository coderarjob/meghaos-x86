/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - User Library - System calls
 * -------------------------------------------------------------------------------------------------
 */
#include <moslib/ulib.h>

S32 syscall (SYSCALLS fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5)
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
    IProcessFlags flags = IPROCESS_FLAGS_NONE;
    if (isKernelMode) {
        flags |= IPROCESS_FLAGS_KERNEL_PROCESS;
    }

    return syscall (SYSCALL_CREATE_PROCESS, (U32)startLocation, binaryLengthBytes, (U32)flags, 0,
                    0);
}

INT sys_thread_create (void (*startLocation)(), bool isKernelMode)
{
    IProcessFlags flags = IPROCESS_FLAGS_THREAD;
    if (isKernelMode) {
        flags |= IPROCESS_FLAGS_KERNEL_PROCESS;
    }
    return syscall (SYSCALL_CREATE_PROCESS, (U32)startLocation, 0, (U32)flags, 0, 0);
}

// This is the entry point for all processes.
__attribute__((section(".entry.text")))
void proc_start() {
    __asm__("jmp proc_main;");
    while (1)
        ;
}
