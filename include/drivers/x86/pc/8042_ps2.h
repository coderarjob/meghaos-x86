/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86, PC Kernel - 8042 PS/2 Controller Header
 * -------------------------------------------------------------------------------------------------
 */

#pragma once

#include <stdbool.h>
#include <types.h>
#include <x86/io.h>

#define PS2_DATA_PORT                (0x60)
#define PS2_CMD_PORT                 (0x64)

// Common PS2 device commands
#define PS2_DEV_CMD_RESET            (0xFF)
#define PS2_DEV_CMD_SET_TO_DEFAULT   (0xF6)
#define PS2_DEV_CMD_IDENTIFY         (0xF2)
#define PS2_DEV_CMD_DISABLE_SCANNING (0xF5)
#define PS2_DEV_CMD_ENABLE_SCANNING  (0xF4)

// Common responses from PS2 devices
#define PS2_DEV_RES_ACK              (0xFA)
#define PS2_DEV_RES_BAT_COMPLETE     (0xAA)
#define PS2_DEV_RES_ECHO             (0xEE)

typedef enum PS2DeviceType {
    PS2_DEVICE_TYPE_UNKNOWN,
    PS2_DEVICE_TYPE_MOUSE,
    PS2_DEVICE_TYPE_KEYBOARD,
} PS2DeviceType;

typedef struct PS2PortInfo PS2PortInfo;
extern PS2PortInfo port1, port2;

bool ps2_init();
bool ps2_canWrite();
bool ps2_canRead();
UINT ps2_read_data();
void ps2_write_data (UINT port, UINT data);
bool ps2_write_device_cmd (const PS2PortInfo* const port, bool checkAck, UINT data);
PS2DeviceType ps2_identify_device (PS2PortInfo* const port);
void ps2_setup_port_configuration (const PS2PortInfo* const port, bool interrupt,
                                   bool kbtranslation);

static inline UINT ps2_read_data_no_wait()
{
    U8 data;
    inb (PS2_DATA_PORT, data);
    return data;
}

static inline void ps2_write_controller_cmd (UINT cmd)
{
    ps2_write_data (PS2_CMD_PORT, cmd);
}
