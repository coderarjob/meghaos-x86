/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - App Library - General functions
 * -------------------------------------------------------------------------------------------------
 */

#include <types.h>
#include <applib/app.h>
#include <applib/syscall.h>

#define OS_MICRODEC_TO_TICK_COUNT(us) ((us) / os_get_tick_period_us())

void event_handler_NDU_()
{
    volatile OSIF_ProcessEvent e = { 0 };
    os_process_pop_event ((OSIF_ProcessEvent*)&e);
    switch (e.event) {
    case OSIF_PROCESS_EVENT_PROCCESS_YIELD_REQ:
        os_yield();
        break;
    case OSIF_PROCESS_EVENT_PROCCESS_CHILD_KILLED:
        INFO ("Child exitted with code: %x", e.data);
        break;
    case OSIF_PROCESS_EVENT_NONE:
        break;
    }
}

void delay (UINT ms)
{
    UINT us = ms * 1000;

    U32 start_tick = os_get_tickcount();
    U32 end_tick   = start_tick + OS_MICRODEC_TO_TICK_COUNT (us);

    while (os_get_tickcount() < end_tick) {
        event_handler_NDU_();
    }
}
