/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86, PC Kernel - 8042 PS/2 Controller Header
 * -------------------------------------------------------------------------------------------------
 */

#pragma once

#include <stdbool.h>
#include <types.h>
#include <x86/io.h>

#define PS2_DATA_PORT                            (0x60)
#define PS2_CMD_PORT                             (0x64)
#define PS2_SPEAKER_PORT                         (0x61)

// PS2 Controller config bit positions
#define PS2_CONFIG_FIRST_PORT_INTERRUPT_ENABLE   (1 << 0U)
#define PS2_CONFIG_SECOND_PORT_INTERRUPT_ENABLE  (1 << 1U)
#define PS2_CONFIG_FIRST_PORT_CLOCK_DISABLE      (1 << 4U)
#define PS2_CONFIG_SECOND_PORT_CLOCK_DISABLE     (1 << 5U)
#define PS2_CONFIG_FIRST_PORT_TRANSLATION_ENABLE (1 << 6U)

// PS2 Controller commands
#define PS2_CMD_DISABLE_SECOND_PORT              (0xA7)
#define PS2_CMD_ENABLE_SECOND_PORT               (0xA8)
#define PS2_CMD_DISABLE_FIRST_PORT               (0xAD)
#define PS2_CMD_ENABLE_FIRST_PORT                (0xAE)
#define PS2_CMD_TEST_CONTROLLER                  (0xAA)
#define PS2_CMD_READ_CONFIGURATION_BYTE          (0x20)
#define PS2_CMD_WRITE_CONFIGURATION_BYTE         (0x60)
#define PS2_CMD_TEST_FIRST_PORT                  (0xAB)
#define PS2_CMD_TEST_SECOND_PORT                 (0xA9)
#define PS2_CMD_SEND_TO_SECOND_PORT              (0xD4)

// Common responses from PS2 controller
#define PS2_RES_TEST_CONTROLLER_PASSED           (0x55)
#define PS2_RES_TEST_CONTROTEST_FAILED           (0xFC)
#define PS2_RES_TEST_PORT_PASSED                 (0x00)

// Common PS2 device commands
#define PS2_DEV_CMD_RESET                        (0xFF)
#define PS2_DEV_CMD_SET_TO_DEFAULT               (0xF6)
#define PS2_DEV_CMD_IDENTIFY                     (0xF2)
#define PS2_DEV_CMD_DISABLE_SCANNING             (0xF5)
#define PS2_DEV_CMD_ENABLE_SCANNING              (0xF4)

// Common responses from PS2 devices
#define PS2_DEV_RES_ACK                          (0xFA)
#define PS2_DEV_RES_RESEND                       (0xFE)

#define PS2_FIRST_DEVICE                         (0U)
#define PS2_SECOND_DEVICE                        (1U)

bool ps2_init();
bool ps2_wait_read (UINT ioport, U8* data);
bool ps2_wait_write (UINT ioport, U8 data);
void ps2_write_device_data_no_ack (UINT device_id, U8 data);
bool ps2_write_device_data_wait_ack (UINT device_id, U8 cmd);
bool ps2_configuration (U8 enabled, U8 disabled, U8* original_config);
INT ps2_identify_device (UINT device_id);

static inline U8 ps2_no_wait_read (UINT ioport)
{
    U8 data;
    inb (ioport, data);
    return data;
}
