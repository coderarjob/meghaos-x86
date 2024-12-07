/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Application - Init program
 * -------------------------------------------------------------------------------------------------
 */

#include <cm.h>
#include <debug.h>

static void init_child_killed (OSIF_ProcessEvent const* const e)
{
#if defined(DEBUG) && defined(PORT_E9_ENABLED)
    CM_DBG_INFO ("Child process exited. Code: %x", e->data);
#else
    (void)e;
#endif
}

void proc_main()
{
    cm_process_register_event_handler (OSIF_PROCESS_EVENT_PROCCESS_CHILD_KILLED, init_child_killed);

    cm_process_create ("MPDEMO.FLT", false);
    while (1) {
        cm_process_handle_events();
    }

    // Should not return!
    // TODO: Place an assert() or panic() here.
    CM_DBG_ERROR ("Init process returning");
}
