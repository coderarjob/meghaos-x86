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

// PS2 Controller commands
#define CMD_DISABLE_SECOND_PORT                  (0xA7)
#define CMD_ENABLE_SECOND_PORT                   (0xA8)
#define CMD_DISABLE_FIRST_PORT                   (0xAD)
#define CMD_ENABLE_FIRST_PORT                    (0xAE)
#define CMD_TEST_CONTROLLER                      (0xAA)
#define CMD_READ_CONFIGURATION_BYTE              (0x20)
#define CMD_WRITE_CONFIGURATION_BYTE             (0x60)
#define CMD_TEST_FIRST_PORT                      (0xAB)
#define CMD_TEST_SECOND_PORT                     (0xA9)
#define CMD_SEND_TO_SECOND_PORT                  (0xD4)

#define RES_TEST_CONTROLLER_PASSED               (0x55)
#define RES_TEST_CONTROTEST_FAILED               (0xFC)
#define RES_TEST_PORT_PASSED                     (0x00)

// PS2 Controller config bit positions
#define CONFIG_FIRST_PORT_INTERRUPT_ENABLE_BIT   (0U)
#define CONFIG_SECOND_PORT_INTERRUPT_ENABLE_BIT  (1U)
#define CONFIG_FIRST_PORT_CLOCK_DISABLE_BIT      (4U)
#define CONFIG_SECOND_PORT_CLOCK_DISABLE_BIT     (5U)
#define CONFIG_FIRST_PORT_TRANSLATION_ENABLE_BIT (6U)

#define IS_READ_READY(status)                    (((status)&0x1) == 1)
#define IS_WRITE_READY(status)                   (((status)&0x2) == 0)

#define RW_TIMEOUT_ITERATION                     200U

struct PS2PortInfo {
    bool isSecondPort;
    bool port_available;
};

PS2PortInfo port1, port2;

bool ps2_canRead()
{
    // Wait for data to be in
    UINT i = 0, status = 0;
    inb (PS2_CMD_PORT, status);
    for (; !IS_READ_READY (status) && i < RW_TIMEOUT_ITERATION; i++) {
        inb (PS2_CMD_PORT, status);
    }
    return (i < RW_TIMEOUT_ITERATION);
}

bool ps2_canWrite()
{
    // Wait for last data to be out
    UINT i = 0, status = 0xFF;
    inb (PS2_CMD_PORT, status);
    for (; !IS_WRITE_READY (status) && i < RW_TIMEOUT_ITERATION; i++) {
        inb (PS2_CMD_PORT, status);
    }
    return (i < RW_TIMEOUT_ITERATION);
}

UINT ps2_read_data()
{
    // Wait for data to be in
    if (ps2_canRead()) {
        return ps2_read_data_no_wait();
    }
    // TODO: Should not panic. It is very much possible for device to ready.
    k_panic ("PS2: Long wait for read");
}

void ps2_write_data (UINT port, UINT data)
{
    // Wait for last data to be out
    if (ps2_canWrite()) {
        outb (port, data);
        return;
    }
    // TODO: Should not panic. It is very much possible for device to ready.
    k_panic ("PS2: Long wait for write");
}

bool ps2_write_device_cmd (const PS2PortInfo* const port, bool checkAck, UINT data)
{
    if (!port->port_available) {
        RETURN_ERROR (ERR_DEVICE_NOT_READY, false);
    }

    if (port->isSecondPort) {
        ps2_write_controller_cmd (CMD_SEND_TO_SECOND_PORT);
    }
    ps2_write_data (PS2_DATA_PORT, data);
    if (checkAck) {
        if (ps2_canRead()) {
            return (ps2_read_data() == PS2_DEV_RES_ACK);
        }
        RETURN_ERROR (ERR_DEVICE_NOT_READY, false);
    }
    return true;
}

PS2DeviceType ps2_identify_device (PS2PortInfo* const port)
{
    k_assert (port != NULL, "PS2 port cannot be null");

    if (!ps2_write_device_cmd (port, true, PS2_DEV_CMD_DISABLE_SCANNING)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, PS2_DEVICE_TYPE_UNKNOWN);
    }

    if (!ps2_write_device_cmd (port, true, PS2_DEV_CMD_IDENTIFY)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, PS2_DEVICE_TYPE_UNKNOWN);
    }

    // Device sends 1 or 2 bytes which indicates its type
    UINT a = ps2_read_data();
    UINT b = ps2_canRead() ? ps2_read_data() : 0U;

    UINT type_num = (UINT)(a << 8U) | b;

    switch (type_num) {
    case 0x0000:
    case 0x0300:
    case 0x0400:
        return PS2_DEVICE_TYPE_MOUSE;
        break;
    case 0xAB83:
    case 0xABC1:
    case 0xAB84:
        return PS2_DEVICE_TYPE_KEYBOARD;
        break;
    default:
        return PS2_DEVICE_TYPE_UNKNOWN;
    }
}

bool ps2_init()
{
    FUNC_ENTRY();

    // Clear the states
    k_memset (&port1, 0, sizeof (port1));
    k_memset (&port2, 0, sizeof (port2));
    port2.isSecondPort = true;

    // Step 1: Disable both PS/2 ports
    // --------------------------------------------------------
    ps2_write_controller_cmd (CMD_DISABLE_FIRST_PORT);
    ps2_write_controller_cmd (CMD_DISABLE_SECOND_PORT);

    // Step 2: Flush the output (from device perspective) buffer
    // --------------------------------------------------------
    U8 discard;
    inb (PS2_DATA_PORT, discard);

    // Step 3: Test PS2 controller
    // --------------------------------------------------------
    ps2_write_controller_cmd (CMD_TEST_CONTROLLER);
    if (ps2_read_data() != RES_TEST_CONTROLLER_PASSED) {
        RETURN_ERROR (ERR_DEVICE_INIT_FAILED, false);
    }

    // Step 4: Determine if there are two channels
    // --------------------------------------------------------
    ps2_write_controller_cmd (CMD_TEST_FIRST_PORT);
    port1.port_available = (ps2_read_data() == RES_TEST_PORT_PASSED);

    ps2_write_controller_cmd (CMD_TEST_SECOND_PORT);
    port2.port_available = (ps2_read_data() == RES_TEST_PORT_PASSED);

    if (!port1.port_available && !port2.port_available) {
        RETURN_ERROR (ERR_DEVICE_INIT_FAILED, false);
    }

    kdebug_println ("port test: port1, port2: %x, %x", port1.port_available, port2.port_available);

    // Step 5: Setup configuration byte
    // --------------------------------------------------------
    ps2_setup_port_configuration (&port1, false, false);
    ps2_setup_port_configuration (&port2, false, false);

    // Step 6: Enable both PS/2 ports
    // --------------------------------------------------------
    ps2_write_controller_cmd (CMD_ENABLE_FIRST_PORT);  // This will turn on the clock for port 1
    ps2_write_controller_cmd (CMD_ENABLE_SECOND_PORT); // This will turn on the clock for port 2

    return true;
}

void ps2_setup_port_configuration (const PS2PortInfo* const port, bool interrupt,
                                   bool kbtranslation)
{
#define BIT_MOD(set, w, b) ((set) ? BIT_SET ((w), (b)) : BIT_CLEAR ((w), (b)))

    FUNC_ENTRY ("port: %px, interrupt: %u, translation: %u", port, interrupt, kbtranslation);

    ps2_write_controller_cmd (CMD_READ_CONFIGURATION_BYTE);
    UINT config = ps2_read_data();

    if (!port->isSecondPort) {
        config = BIT_MOD (interrupt, config, CONFIG_FIRST_PORT_INTERRUPT_ENABLE_BIT);
        config = BIT_MOD (kbtranslation, config, CONFIG_FIRST_PORT_TRANSLATION_ENABLE_BIT);
    } else {
        config = BIT_MOD (interrupt, config, CONFIG_SECOND_PORT_INTERRUPT_ENABLE_BIT);
    }

    ps2_write_controller_cmd (CMD_WRITE_CONFIGURATION_BYTE);
    ps2_write_data (PS2_DATA_PORT, config);
    kdebug_println ("Config written: %x", config);
}
