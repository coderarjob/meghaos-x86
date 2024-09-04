/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86, PC Kernel - 8254 PIT dirver headers
 * -------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>
#define X86_PIT_COUNTER_MODE2 (2U)
#define X86_PIT_COUNTER_MODE3 (3U)

void pit_get_interrupt_counter (U8* mode, U16* value);
void pit_set_interrupt_counter (U8 mode, U16 value);
void pit_set_speaker_counter (U16 value);
