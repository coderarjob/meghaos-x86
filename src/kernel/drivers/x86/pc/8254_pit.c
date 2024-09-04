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

#define COUNTER0                     (0U)
#define COUNTER2                     (2U)

#define CONTROLWORD_RW_TWO_BYTES     (3U)

#define CONTROLWORD(cntr, rw, mode)  (((cntr) << 6U) | ((rw) << 4U) | ((mode) << 1U))

#define CONTROLWORD_READBACK_COMMAND (3U)
#define READBACK(cntr)               ((CONTROLWORD_READBACK_COMMAND << 6U) | (1 << ((cntr + 1))))
#define READBACK_STATUS_MODE_MASK    (0xEU)

static void set_counter (UINT cntr, UINT mode, U16 value)
{
    k_assert (cntr == COUNTER0 || cntr == COUNTER2, "Invalid counter");
    k_assert (mode == X86_PIT_COUNTER_MODE2 || mode == X86_PIT_COUNTER_MODE3, "Invalid counter");

    UINT port  = (cntr == COUNTER0) ? COUNTER0_PORT : COUNTER2_PORT;
    U8 byteLow = (value & 0xFFU), byteHigh = ((value >> 8U) & 0xFFU);

    // Write control word
    UINT cw = CONTROLWORD (cntr, CONTROLWORD_RW_TWO_BYTES, mode);
    outb (CONTROLWORD_PORT, cw);
    // Write initial count
    outb (port, byteLow);
    outb (port, byteHigh);

    // INFO ("CWord: %x, port: %x, bytehigh: %x, bytelow: %x", cw, port, byteHigh, byteLow);
}

static void get_counter (UINT cntr, U8* status, U16* value)
{
    k_assert (status != NULL && value != NULL, "Invalid input");
    k_assert (cntr == COUNTER0 || cntr == COUNTER2, "Invalid counter");
    UINT port = (cntr == COUNTER0) ? COUNTER0_PORT : COUNTER2_PORT;

    //// Send controlword readback command
    UINT cw = READBACK (cntr);
    outb (CONTROLWORD_PORT, cw);

    //// First byte is the status
    U8 byteLow, byteHigh;
    inb (port, *status);
    inb (port, byteLow);
    inb (port, byteHigh);

    *value = (U16)((byteHigh << 8U) | byteLow);

    // INFO ("CWord: %x, port: %x, status: %x, high: %x, low: %x", cw, port, *status, byteHigh,
    // byteLow);
}

void pit_get_interrupt_counter (U8* mode, U16* value)
{
    FUNC_ENTRY ("mode:  %px, value: %px", mode, value);

    U8 status;
    get_counter (COUNTER0, &status, value);
    *mode = (status & READBACK_STATUS_MODE_MASK) >> 1;
}

void pit_set_interrupt_counter (U8 mode, U16 value)
{
    FUNC_ENTRY ("mode:  %x, value: %x", mode, value);
    set_counter (COUNTER0, mode, value);
}

void pit_set_speaker_counter (U16 value)
{
    FUNC_ENTRY ("value: %x", value);
    set_counter (COUNTER2, X86_PIT_COUNTER_MODE3, value);
}

void pit_stop_start_counter (U8 cntr, bool doStop)
{
    FUNC_ENTRY ("counter: %x, doStop:  %u", cntr, doStop);
    UNREACHABLE();
}
