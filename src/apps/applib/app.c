/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - App Library - General functions
 * -------------------------------------------------------------------------------------------------
 */

#include <types.h>
#include <applib/app.h>
#include <applib/syscall.h>

#define OS_MICRODEC_TO_TICK_COUNT(us) ((us) / os_get_tick_period_us())

void delay (UINT ms)
{
    UINT us = ms * 1000;

    U32 start_tick = os_get_tickcount();
    U32 end_tick   = start_tick + OS_MICRODEC_TO_TICK_COUNT (us);

    while (os_get_tickcount() < end_tick) {
        if (os_process_is_yield_requested()) {
            os_yield();
        }
    }
}
