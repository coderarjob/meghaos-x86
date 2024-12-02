/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86, PC Kernel - PS/2 Devices
 * -------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>

#define PS2_MOUSE_CMD_GET_ID                               (0xF2)
#define PS2_MOUSE_CMD_STATUS_REQ                           (0xE9)
#define PS2_MOUSE_CMD_SET_SAMPLE_RATE                      (0xF3)
#define PS2_MOUSE_CMD_GET_MOUSE_PACKET                     (0xEB)
#define PS2_MOUSE_CMD_ENTER_REMOTE_MODE                    (0xF0)
#define PS2_MOUSE_CMD_ENTER_STREAM_MODE                    (0xEA)

#define PS2_MOUSE_PACKET_BYTE0_LEFT_BTN_MASK               (1 << 0)
#define PS2_MOUSE_PACKET_BYTE0_RIGHT_BTN_MASK              (1 << 1)
#define PS2_MOUSE_PACKET_BYTE0_MID_BTN_MASK                (1 << 2)
#define PS2_MOUSE_PACKET_BYTE0_X_SIGN_BTN_MASK             (1 << 4)
#define PS2_MOUSE_PACKET_BYTE0_Y_SIGN_BTN_MASK             (1 << 5)
#define PS2_MOUSE_PACKET_BYTE0_X_OVERFLOW_BTN_MASK         (1 << 6)
#define PS2_MOUSE_PACKET_BYTE0_Y_OVERFLOW_BTN_MASK         (1 << 7)

#define PS2_MOUSE_STATE_BYTE_0_RIGHT_BTN_MASK              (1 << 0)
#define PS2_MOUSE_STATE_BYTE_0_MID_BTN_MASK                (1 << 1)
#define PS2_MOUSE_STATE_BYTE_0_LEFT_BTN_MASK               (1 << 2)
#define PS2_MOUSE_STATE_BYTE_0_SCALE_2x1_ENABLED_MASK      (1 << 4)
#define PS2_MOUSE_STATE_BYTE_0_DATA_REPORTING_ENABLED_MASK (1 << 5)
#define PS2_MOUSE_STATE_BYTE_0_REMOTE_MODE_ENABLED         (1 << 6)

typedef struct MousePositionData {
    INT x;
    INT y;
    bool left_button;
    bool right_button;
    bool middle_button;
} MousePositionData;

bool ps2_mouse_init();
bool ps2_kb_init();
MousePositionData ps2_mouse_get_packet();
