/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - App Library - General functions
 * -------------------------------------------------------------------------------------------------
 */

#include <types.h>
#include <cm/cm.h>
#include <cm/syscall.h>
#include <cm/debug.h>
#include <kcmlib.h>

/* Variable to store Library error*/
uint32_t cm_error_num;

#define cm_MICRODEC_TO_TICK_COUNT(us) ((us) / cm_get_tick_period_us())

/***************************************************************************************************
 * Halts thread for 'ms' miliseconds
 *
 * @return      Nothing
 **************************************************************************************************/
void cm_delay (UINT ms)
{
    UINT us = ms * 1000;

    U32 start_tick = cm_get_tickcount();
    U32 end_tick   = start_tick + cm_MICRODEC_TO_TICK_COUNT (us);

    while (cm_get_tickcount() < end_tick) {
        cm_process_handle_events();
    }
}

/***************************************************************************************************
 * Return the last library error
 *
 * @return      Last libcm error
 **************************************************************************************************/
uint32_t cm_get_lib_error()
{
    return cm_error_num;
}
