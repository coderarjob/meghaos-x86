/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86, PC Kernel - PC Speaker
 * -------------------------------------------------------------------------------------------------
 */

#include <types.h>
#include <kdebug.h>
#include <drivers/x86/pc/8042_ps2.h>
#include <drivers/x86/pc/8254_pit.h>
#include <drivers/x86/pc/pcspeaker.h>

void pcspeaker_set_freq (UINT freq)
{
    FUNC_ENTRY ("frequency Hz: %x", freq);

    U16 value = (U16)((double)PIT_BASE_CLOCK_FREQ_HZ / freq);
    pit_set_speaker_counter (value);
}

void pcspeaker_turnon()
{
    FUNC_ENTRY();

    U8 data = ps2_no_wait_read (PS2_SPEAKER_PORT);
    data |= 3U;
    ps2_wait_write (PS2_SPEAKER_PORT, data);
}

void pcspeaker_turnoff()
{
    FUNC_ENTRY();
    U8 data = ps2_no_wait_read (PS2_SPEAKER_PORT);
    data |= 3U;
    ps2_wait_write (PS2_SPEAKER_PORT, data);
}
