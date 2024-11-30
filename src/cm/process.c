/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - C MOS App Library - Process management
 * -------------------------------------------------------------------------------------------------
 */

#include <process.h>
#include <cm/syscall.h>
#include <cm/err.h>

INT cm_process_create (void* startLocation, SIZE binaryLengthBytes, bool isKernelMode)
{
    if (!startLocation || binaryLengthBytes == 0) {
        CM_RETURN_ERROR__ (CM_ERR_INVALID_INPUT, CM_FAILURE);
    }

    KProcessFlags flags = PROCESS_FLAGS_NONE;
    if (isKernelMode) {
        flags |= PROCESS_FLAGS_KERNEL_PROCESS;
    }

    INT pid = syscall (OSIF_SYSCALL_CREATE_PROCESS, (U32)startLocation, binaryLengthBytes,
                       (U32)flags, 0, 0);
    if (pid < 0) {
        CM_RETURN_ERROR__ (cm_get_os_error(), CM_FAILURE);
    }
    return pid;
}

INT cm_thread_create (void (*startLocation)(), bool isKernelMode)
{
    if (!startLocation) {
        CM_RETURN_ERROR__ (CM_ERR_INVALID_INPUT, CM_FAILURE);
    }

    KProcessFlags flags = PROCESS_FLAGS_THREAD;
    if (isKernelMode) {
        flags |= PROCESS_FLAGS_KERNEL_PROCESS;
    }

    INT pid = syscall (OSIF_SYSCALL_CREATE_PROCESS, (U32)startLocation, 0, (U32)flags, 0, 0);
    if (pid < 0) {
        CM_RETURN_ERROR__ (cm_get_os_error(), CM_FAILURE);
    }
    return pid;
}

bool cm_process_is_yield_requested()
{
    volatile OSIF_ProcessEvent e = { 0 };
    cm_process_pop_event ((OSIF_ProcessEvent*)&e);
    return (e.event == OSIF_PROCESS_EVENT_PROCCESS_YIELD_REQ);
}
