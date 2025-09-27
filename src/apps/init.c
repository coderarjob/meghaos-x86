/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Application - Init program
 * -------------------------------------------------------------------------------------------------
 */

#include <cm.h>
#include <debug.h>
#include <err.h>

static void init_child_killed (OSIF_ProcessEvent const* const e)
{
    CM_DBG_INFO ("Child process exited. Code: %x", e->data);
    (void)e;
}

void proc_main(void)
{
    cm_process_register_event_handler (OSIF_PROCESS_EVENT_PROCCESS_CHILD_KILLED, init_child_killed);

    cm_process_create (INIT_PROG, false);

    while (1) {
        cm_process_handle_events();
    }

    // Should not return!
    cm_panic();
}
