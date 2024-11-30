/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - App Library - General functions
 * -------------------------------------------------------------------------------------------------
 */

#include <types.h>
#include <cm/cm.h>
#include <cm/syscall.h>
#include <cm/debug.h>

/* Variable to store Library error*/
uint32_t cm_error_num__;

#define cm_MICRODEC_TO_TICK_COUNT(us) ((us) / cm_get_tick_period_us())

void event_handler_NDU_()
{
    volatile OSIF_ProcessEvent e = { 0 };
    cm_process_pop_event ((OSIF_ProcessEvent*)&e);
    switch (e.event) {
    case OSIF_PROCESS_EVENT_PROCCESS_YIELD_REQ:
        cm_process_yield();
        break;
    case OSIF_PROCESS_EVENT_PROCCESS_CHILD_KILLED:
        CM_DBG_INFO ("Child exitted with code: %x", e.data);
        break;
    case OSIF_PROCESS_EVENT_NONE:
        break;
    }
}

void cm_delay (UINT ms)
{
    UINT us = ms * 1000;

    U32 start_tick = cm_get_tickcount();
    U32 end_tick   = start_tick + cm_MICRODEC_TO_TICK_COUNT (us);

    while (cm_get_tickcount() < end_tick) {
        event_handler_NDU_();
    }
}
