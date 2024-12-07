/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - C MOS App Library - Process management
 * -------------------------------------------------------------------------------------------------
 */

#include <process.h>
#include <cm/syscall.h>
#include <cm/err.h>
#include <cm/cm.h>
#include <stdbool.h>

/***************************************************************************************************
 * Handling of process events
 * Some events are handled automaticaly by the library while others are for the applications to
 * handle. The applications can register a hander function for each event and those will be called
 * when the associated event is read.
 *
 * Not that though every thread receives events, the application event handler function is common
 * and is shared by every thread & the parent process. Per thread handling of events can be
 * done by quering process id.
 ***************************************************************************************************/
static cm_event_handler app_event_handlers[OSIF_PROCESS_EVENTS_COUNT] = { 0 };

bool cm_process_register_event_handler (OSIF_ProcessEvents e, cm_event_handler h)
{
    if (app_event_handlers[e] != NULL) {
        CM_RETURN_ERROR__ (CM_ERR_EVENT_HANDLER_ALREADY_REGISTERED, false);
    }

    app_event_handlers[e] = h;
    return true;
}

/***************************************************************************************************
* Default handler for process events
*
* Note:
* Since its upto each process to read their events queue and handle them,
* it is important that cm_process_handle_events() gets called regularly.
**************************************************************************************************/
bool cm_process_handle_events()
{
    volatile OSIF_ProcessEvent e = { 0 };
    if (!cm_process_pop_event ((OSIF_ProcessEvent*)&e)) {
        CM_RETURN_ERROR__ (cm_get_os_error(), false);
    }

    switch (e.event) {
    case OSIF_PROCESS_EVENT_PROCCESS_YIELD_REQ:
        if (app_event_handlers[e.event]) {
            app_event_handlers[e.event]((const OSIF_ProcessEvent* const)&e);
        }
        cm_process_yield();
        break;
    case OSIF_PROCESS_EVENT_PROCCESS_CHILD_KILLED:
        if (app_event_handlers[e.event]) {
            app_event_handlers[e.event]((const OSIF_ProcessEvent* const)&e);
        }
        break;
    case OSIF_PROCESS_EVENT_NONE:
        break;
    default:
        // TODO: Should panic! and kill the process
        break;
    }
    return true;
}
/**************************************************************************************************/

INT cm_process_create (const char* const filename, bool isKernelMode)
{
    if (filename == NULL) {
        CM_RETURN_ERROR__ (CM_ERR_INVALID_INPUT, CM_FAILURE);
    }

    KProcessFlags flags = PROCESS_FLAGS_NONE;
    if (isKernelMode) {
        flags |= PROCESS_FLAGS_KERNEL_PROCESS;
    }

    OSIF_BootLoadedFiles file = { 0 };
    cm_get_bootloaded_file (filename, &file);

    INT pid = syscall (OSIF_SYSCALL_CREATE_PROCESS, (U32)file.startLocation, file.length,
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
