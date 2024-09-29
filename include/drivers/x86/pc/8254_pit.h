/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86, PC Kernel - 8254 PIT dirver headers
 * -------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>
#include <kassert.h>
#include <moslimits.h>

#define PIT_BASE_CLOCK_FREQ_HZ 1193182

typedef enum PITCounterModes {
    PIT_COUNTER_MODE_2 = 2,
    PIT_COUNTER_MODE_3 = 3
} PITCounterModes;

typedef enum X86_PITCounters {
    PIT_COUNTER_0 = 0,
    PIT_COUNTER_2 = 2
} PITCounters;

void pit_get_counter (PITCounters cntr, U8* status, U16* value);
void pit_set_counter (PITCounters cntr, PITCounterModes mode, U16 value);
void pit_get_interrupt_counter (PITCounterModes* mode, U16* value);
void pit_stop_start_counter (PITCounters cntr, bool turnOn);

static inline void pit_set_interrupt_counter (PITCounterModes mode, UINT freq)
{
    k_assert (freq >= MIN_INTERRUPT_CLOCK_FREQ_HZ, "Freq too small");
    pit_set_counter (PIT_COUNTER_0, mode, (U16)(PIT_BASE_CLOCK_FREQ_HZ / freq));
}

static inline void pit_set_speaker_counter (UINT freq)
{
    k_assert (freq >= MIN_INTERRUPT_CLOCK_FREQ_HZ, "Freq too small");
    pit_set_counter (PIT_COUNTER_2, PIT_COUNTER_MODE_3, (U16)(PIT_BASE_CLOCK_FREQ_HZ / freq));
}
