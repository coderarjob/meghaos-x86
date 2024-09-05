/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86, PC Kernel - 8042 PS/2 Controller
 * -------------------------------------------------------------------------------------------------
 */

#include <drivers/x86/pc/8042_ps2.h>
#include <types.h>
#include <kdebug.h>
#include <x86/io.h>
#include <kassert.h>

#define PS2_DATA_PORT                    0x60
#define PS2_CMD_PORT                     0x64

#define PS2_CMD_DISABLE_SECOND_PORT      0xA7
#define PS2_CMD_ENABLE_SECOND_PORT       0xA8

#define PS2_CMD_DISABLE_FIRST_PORT       0xAD
#define PS2_CMD_ENABLE_FIRST_PORT        0xAE
#define PS2_CMD_TEST_CONTROLLER          0xAA
#define PS2_CMD_READ_CONFIGURATION_BYTE  0x20
#define PS2_CMD_WRITE_CONFIGURATION_BYTE 0x60
#define PS2_CMD_TEST_FIRST_PORT          0xAB
#define PS2_CMD_TEST_SECOND_PORT         0xA9
#define PS2_CMD_RESET_PORT               0xFF

#define PS2_RES_TEST_CONTROLLER_PASSED   0x55
#define PS2_RES_TEST_CONTROTEST_FAILED   0xFC
#define PS2_RES_TEST_PORT_PASSED         0x00
#define PS2_RES_RESET_SUCCESS_BYTE0      0xFA
#define PS2_RES_RESET_SUCCESS_BYTE1      0xAA

#define IS_READ_READY(status)            (((status)&0x1) == 1)
#define IS_WRITE_READY(status)           (((status)&0x2) == 0)

#define RW_TIMEOUT_ITERATION             1000U

// Enable interrupt, clock & translation for 1st PS2 port
#define PS2_CONFIG_FIRST_PORT            (0x51)

// Enable Interrupt, clock for 2nd PS2 port
#define PS2_CONFIG_SECOND_PORT           (0x22)

#define IS_DEVICE_RESET_SUCCESS(a, b)                                                  \
    ((((a) == PS2_RES_RESET_SUCCESS_BYTE0) && ((b) == PS2_RES_RESET_SUCCESS_BYTE1)) || \
     (((a) == PS2_RES_RESET_SUCCESS_BYTE1) && ((b) == PS2_RES_RESET_SUCCESS_BYTE0)))

bool ps2_port_1_available, ps2_port_2_available;

static UINT read_port (UINT port)
{
    // Wait for data to be in
    UINT i = 0, status = 0;
    inb (PS2_CMD_PORT, status);
    for (; !IS_READ_READY (status) && i < RW_TIMEOUT_ITERATION; i++) {
        inb (PS2_CMD_PORT, status);
    }
    k_assert (i < RW_TIMEOUT_ITERATION, "PS2: Long wait");

    U8 value;
    inb (port, value);
    return value;
}

static void write_port (UINT port, UINT data)
{
    // Wait for last data to be out
    UINT i = 0, status = 0xFF;
    inb (PS2_CMD_PORT, status);
    for (; !IS_WRITE_READY (status) && i < RW_TIMEOUT_ITERATION; i++) {
        inb (PS2_CMD_PORT, status);
    }
    k_assert (i < RW_TIMEOUT_ITERATION, "PS2: Long wait");

    outb (port, data);
}

void ps2_init()
{
    FUNC_ENTRY();

    UINT result;

    // TODO: Check ACPI tables to see if PS/2 controller exists. For now I will continue with the
    // assumption that it does at least writing to the ports will not cause a failure if it does not
    // exists.

    // Disable both PS/2 ports
    write_port (PS2_CMD_PORT, PS2_CMD_DISABLE_FIRST_PORT);
    write_port (PS2_CMD_PORT, PS2_CMD_DISABLE_SECOND_PORT);

    // Flush the output (from device perspective) buffer
    U8 discard;
    inb (PS2_DATA_PORT, discard);

    // Perform Controller self test
    write_port (PS2_CMD_PORT, PS2_CMD_TEST_CONTROLLER);
    result = read_port (PS2_DATA_PORT);
    k_assert (result == PS2_RES_TEST_CONTROLLER_PASSED, "PS2: Self test failed");

    // Test PS/2 ports (not the devices) & determine which port is working
    write_port (PS2_CMD_PORT, PS2_CMD_TEST_FIRST_PORT);
    ps2_port_1_available = (read_port (PS2_DATA_PORT) == PS2_RES_TEST_PORT_PASSED);

    write_port (PS2_CMD_PORT, PS2_CMD_TEST_SECOND_PORT);
    ps2_port_2_available = (read_port (PS2_DATA_PORT) == PS2_RES_TEST_PORT_PASSED);

    k_assert (ps2_port_1_available || ps2_port_2_available, "PS2: No ports working.");

    // Configure PS2 and enable ports
    write_port (PS2_CMD_PORT, PS2_CMD_READ_CONFIGURATION_BYTE);
    UINT config = read_port (PS2_DATA_PORT);
    if (ps2_port_1_available) {
        write_port (PS2_CMD_PORT, PS2_CMD_ENABLE_FIRST_PORT);
        config |= PS2_CONFIG_FIRST_PORT;
    }
    if (ps2_port_2_available) {
        write_port (PS2_CMD_PORT, PS2_CMD_ENABLE_SECOND_PORT);
        config |= PS2_CONFIG_SECOND_PORT;
    }
    write_port (PS2_CMD_PORT, PS2_CMD_WRITE_CONFIGURATION_BYTE);
    write_port (PS2_DATA_PORT, config);
    kdebug_println("Config written: %x", config);

    // Reset device (Port 1)
    if (ps2_port_1_available) {
        write_port (PS2_DATA_PORT, PS2_CMD_RESET_PORT);
        result = read_port (PS2_DATA_PORT); kdebug_println ("result (1st): %x", result);
        if (result == PS2_RES_RESET_SUCCESS_BYTE0 || result == PS2_RES_RESET_SUCCESS_BYTE1) {
            // On success first byte is either 0xFA or 0xAA. Got that
            result = read_port (PS2_DATA_PORT); kdebug_println ("result (2st): %x", result);
            if (result == PS2_RES_RESET_SUCCESS_BYTE0 || result == PS2_RES_RESET_SUCCESS_BYTE1) {
                //result = read_port (PS2_DATA_PORT); kdebug_println ("result (3rd): %x", result);
            }
        }
    }

    if (ps2_port_2_available) {
        write_port (PS2_CMD_PORT, 0xD4);
        write_port (PS2_DATA_PORT, PS2_CMD_RESET_PORT);
        result = read_port (PS2_DATA_PORT); kdebug_println ("result (1st): %x", result);
        if (result == PS2_RES_RESET_SUCCESS_BYTE0 || result == PS2_RES_RESET_SUCCESS_BYTE1) {
            // On success first byte is either 0xFA or 0xAA. Got that
            result = read_port (PS2_DATA_PORT); kdebug_println ("result (2st): %x", result);
            if (result == PS2_RES_RESET_SUCCESS_BYTE0 || result == PS2_RES_RESET_SUCCESS_BYTE1) {
                //result = read_port (PS2_DATA_PORT); kdebug_println ("result (3rd): %x", result);
            }
        }
    }
    //write_port (PS2_CMD_PORT, PS2_CMD_READ_CONFIGURATION_BYTE);
    //config = read_port (PS2_DATA_PORT);
    //INFO("Config read 2: %x", config);
}
