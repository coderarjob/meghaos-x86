/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86, PC Kernel - 8042 PS/2 Controller
 * -------------------------------------------------------------------------------------------------
 */

#include <drivers/x86/pc/8042_ps2.h>
#include <stdbool.h>
#include <types.h>
#include <kdebug.h>
#include <x86/io.h>
#include <kassert.h>
#include <utils.h>
#include <kerror.h>
#include <kstdlib.h>
#include <x86/cpu.h>

#define IS_READ_READY(status)  (((status)&0x1) == 1)
#define IS_WRITE_READY(status) (((status)&0x2) == 0)

#define RW_TIMEOUT_ITERATION   (200U)
#define DEVICE_COUNT           (2U)

typedef struct PS2Device {
    bool isSecondPort;
    bool port_available;
} PS2Device;

PS2Device devices[DEVICE_COUNT];

static bool ps2_canRead()
{
    // Wait for data to be in
    UINT i = 0, status = 0;
    inb (PS2_CMD_PORT, status);
    for (; !IS_READ_READY (status) && i < RW_TIMEOUT_ITERATION; i++) {
        inb (PS2_CMD_PORT, status);
        X86_PAUSE();
    }

    return (i < RW_TIMEOUT_ITERATION);
}

bool ps2_wait_read (UINT ioport, U8* data)
{
    // Wait for data to be in
    if (!ps2_canRead()) {
        RETURN_ERROR (ERR_TIMEOUT, false);
    }

    inb (ioport, *data);
    return true;
}

bool ps2_wait_write (UINT ioport, U8 data)
{
    // Wait for last data to be out
    UINT i = 0, status = 0xFF;
    inb (PS2_CMD_PORT, status);
    for (; !IS_WRITE_READY (status) && i < RW_TIMEOUT_ITERATION; i++) {
        inb (PS2_CMD_PORT, status);
        X86_PAUSE();
    }
    if (i == RW_TIMEOUT_ITERATION) {
        RETURN_ERROR (ERR_TIMEOUT, false);
    }

    outb (ioport, data);
    return true;
}

void ps2_write_device_data (UINT device_id, U8 data)
{
    k_assert (device_id < DEVICE_COUNT, "Device ID is invalid.");

    const PS2Device* const device = &devices[device_id];

    k_assert (device->port_available, "Device not ready");

    if (device->isSecondPort) {
        ps2_wait_write (PS2_CMD_PORT, PS2_CMD_SEND_TO_SECOND_PORT);
    }

    ps2_wait_write (PS2_DATA_PORT, data);
}

bool ps2_write_device_cmd (UINT device_id, U8 cmd)
{
    U8 ack, retrycount = 0;

    do {
        ps2_write_device_data (device_id, cmd);
        if (!ps2_wait_read (PS2_DATA_PORT, &ack)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, false); // Possible timeout
        }
    } while (ack == 0xFE && retrycount++ < 3);

    if (ack != PS2_DEV_RES_ACK) {
        RETURN_ERROR (ERR_DEVICE_NOT_READY, false);
    }

    return true;
}

bool ps2_configuration (U8 enabled, U8 disabled, U8* original_config)
{
    U8 config;
    if (!ps2_wait_write (PS2_CMD_PORT, PS2_CMD_READ_CONFIGURATION_BYTE)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }
    if (!ps2_wait_read (PS2_DATA_PORT, &config)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    kdebug_println ("Config read: %x", config);

    // Return the read config if point is not NULL
    if (original_config != NULL) {
        *original_config = config;
    }

    // Change the config as requested
    config &= (U8) ~(disabled);
    config |= (enabled);

    kdebug_println ("Config written: %x", config);

    // Write back the modified config
    if (!ps2_wait_write (PS2_CMD_PORT, PS2_CMD_WRITE_CONFIGURATION_BYTE)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }
    if (!ps2_wait_write (PS2_DATA_PORT, config)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }
    return true;
}

INT ps2_identify_device (UINT device_id)
{
    k_assert (device_id < DEVICE_COUNT, "Device ID is invalid.");

    U8 config;
    ps2_configuration (0,
                       PS2_CONFIG_FIRST_PORT_INTERRUPT_ENABLE |
                           PS2_CONFIG_SECOND_PORT_INTERRUPT_ENABLE |
                           PS2_CONFIG_FIRST_PORT_TRANSLATION_ENABLE,
                       &config);

    if (!ps2_write_device_cmd (device_id, PS2_DEV_CMD_DISABLE_SCANNING)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE);
    }

    if (!ps2_write_device_cmd (device_id, PS2_DEV_CMD_IDENTIFY)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE);
    }

    // Device sends 1 or 2 bytes which indicates its type
    U8 byte0, byte1;
    byte0 = byte1 = 0;

    if (!ps2_wait_read (PS2_DATA_PORT, &byte0)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, KERNEL_EXIT_FAILURE);
    }
    if (ps2_canRead()) {
        ps2_wait_read (PS2_DATA_PORT, &byte1);
    }

    kdebug_println ("identity: byte0, byte1: %x %x", byte0, byte1);

    // Restore configuration
    ps2_configuration (config, 0, NULL);

    return (byte1 << 8U) | byte0;
}

bool ps2_init()
{
    FUNC_ENTRY();

    U8 config;

    // Clear the states
    k_memset (devices, 0, sizeof (PS2Device) * DEVICE_COUNT);
    devices[1].isSecondPort = true;

    // Step 1: Disable both PS/2 ports
    // --------------------------------------------------------
    if (!ps2_wait_write (PS2_CMD_PORT, PS2_CMD_DISABLE_FIRST_PORT)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    if (!ps2_wait_write (PS2_CMD_PORT, PS2_CMD_DISABLE_SECOND_PORT)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    // Step 2: Flush the output (from device perspective) buffer
    // --------------------------------------------------------
    while (ps2_canRead()) {
        (void)ps2_no_wait_read (PS2_DATA_PORT);
    }

    // Step 3: Setup configuration byte
    // --------------------------------------------------------
    ps2_configuration (0,
                       PS2_CONFIG_FIRST_PORT_INTERRUPT_ENABLE |
                           PS2_CONFIG_SECOND_PORT_INTERRUPT_ENABLE |
                           PS2_CONFIG_FIRST_PORT_TRANSLATION_ENABLE,
                       NULL);

    // Step 4: Test PS2 controller
    // --------------------------------------------------------
    ps2_wait_write (PS2_CMD_PORT, PS2_CMD_TEST_CONTROLLER);
    if (!ps2_wait_read (PS2_DATA_PORT, &config)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }
    if (config != PS2_RES_TEST_CONTROLLER_PASSED) {
        RETURN_ERROR (ERR_DEVICE_INIT_FAILED, false);
    }

    // Step 5: Determine if there are two channels
    // --------------------------------------------------------
    ps2_wait_write (PS2_CMD_PORT, PS2_CMD_TEST_FIRST_PORT);
    if (ps2_wait_read (PS2_DATA_PORT, &config)) {
        devices[0].port_available = (config == PS2_RES_TEST_PORT_PASSED);
    }

    ps2_wait_write (PS2_CMD_PORT, PS2_CMD_TEST_SECOND_PORT);
    if (ps2_wait_read (PS2_DATA_PORT, &config)) {
        devices[1].port_available = (config == PS2_RES_TEST_PORT_PASSED);
    }

    if (!devices[0].port_available && !devices[1].port_available) {
        RETURN_ERROR (ERR_DEVICE_INIT_FAILED, false);
    }

    kdebug_println ("port test: port1, port2: %x, %x", devices[0].port_available,
                    devices[1].port_available);

    // Step 6: Enable both PS/2 ports
    // --------------------------------------------------------
    ps2_wait_write (PS2_CMD_PORT,
                    PS2_CMD_ENABLE_FIRST_PORT); // This will turn on the clock for port 1
    ps2_wait_write (PS2_CMD_PORT,
                    PS2_CMD_ENABLE_SECOND_PORT); // This will turn on the clock for port 2

    return true;
}
