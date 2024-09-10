/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86, PC Kernel - 8254 PIT dirver
 *
 * Note:
 * - Channel 1 cannot be programmed
 * - Mode 2 and 3 are the only ones that are supported
 * - Read-Back and 2 bytes for programming is the only mode R/W mode supported.
 * -------------------------------------------------------------------------------------------------
 */

#include <types.h>
#include <kdebug.h>
#include <kassert.h>
#include <x86/io.h>
#include <utils.h>
#include <drivers/x86/pc/8254_pit.h>

#define COUNTER0_PORT                0x40
#define COUNTER2_PORT                0x42
#define CONTROLWORD_PORT             0x43

#define CONTROLWORD_RW_TWO_BYTES     (3U)

#define CONTROLWORD(cntr, rw, mode)  (((cntr) << 6U) | ((rw) << 4U) | ((mode) << 1U))

#define CONTROLWORD_READBACK_COMMAND (3U)
#define READBACK(cntr)               ((CONTROLWORD_READBACK_COMMAND << 6U) | (1 << ((cntr + 1))))
#define READBACK_STATUS_MODE_MASK    (0xEU)

void pit_set_counter (PITCounters cntr, PITCounterModes mode, U16 value)
{
    FUNC_ENTRY ("counter: %x, mode: %x, value: %x", cntr, mode, value);

    k_assert (cntr == PIT_COUNTER_0 || cntr == PIT_COUNTER_2, "Invalid counter");
    k_assert (mode == PIT_COUNTER_MODE_2 || mode == PIT_COUNTER_MODE_3, "Invalid counter");

    UINT port  = (cntr == PIT_COUNTER_0) ? COUNTER0_PORT : COUNTER2_PORT;
    U8 byteLow = (value & 0xFFU), byteHigh = ((value >> 8U) & 0xFFU);

    // Write control word
    UINT cw = CONTROLWORD (cntr, CONTROLWORD_RW_TWO_BYTES, mode);
    outb (CONTROLWORD_PORT, cw);
    // Write initial count
    outb (port, byteLow);
    outb (port, byteHigh);
}

void pit_get_counter (PITCounters cntr, U8* status, U16* value)
{
    FUNC_ENTRY ("counter: %x, status: %px, value: %px", cntr, status, value);

    k_assert (status != NULL && value != NULL, "Invalid input");
    k_assert (cntr == PIT_COUNTER_0 || cntr == PIT_COUNTER_2, "Invalid counter");
    UINT port = (cntr == PIT_COUNTER_0) ? COUNTER0_PORT : COUNTER2_PORT;

    //// Send controlword readback command
    UINT cw = READBACK (cntr);
    outb (CONTROLWORD_PORT, cw);

    //// First byte is the status
    U8 byteLow, byteHigh;
    inb (port, *status);
    inb (port, byteLow);
    inb (port, byteHigh);

    *value = (U16)((byteHigh << 8U) | byteLow);
}

void pit_get_interrupt_counter (PITCounterModes* mode, U16* value)
{
    FUNC_ENTRY ("mode:  %px, value: %px", mode, value);

    U8 status;
    pit_get_counter (PIT_COUNTER_0, &status, value);
    *mode = (status & READBACK_STATUS_MODE_MASK) >> 1;
}
