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

#define DATA_PORT                                (0x60)
#define CMD_PORT                                 (0x64)

#define CMD_DISABLE_SECOND_PORT                  (0xA7)
#define CMD_ENABLE_SECOND_PORT                   (0xA8)
#define CMD_DISABLE_FIRST_PORT                   (0xAD)
#define CMD_ENABLE_FIRST_PORT                    (0xAE)
#define CMD_TEST_CONTROLLER                      (0xAA)
#define CMD_READ_CONFIGURATION_BYTE              (0x20)
#define CMD_WRITE_CONFIGURATION_BYTE             (0x60)
#define CMD_TEST_FIRST_PORT                      (0xAB)
#define CMD_TEST_SECOND_PORT                     (0xA9)
#define CMD_RESET_PORT                           (0xFF)
#define CMD_SEND_TO_SECOND_PORT                  (0xD4)

#define RES_TEST_CONTROLLER_PASSED               (0x55)
#define RES_TEST_CONTROTEST_FAILED               (0xFC)
#define RES_TEST_PORT_PASSED                     (0x00)

#define DEV_CMD_RESET                            (0xFF)
#define DEV_CMD_SET_TO_DEFAULT                   (0xF6)
#define DEV_CMD_IDENTIFY                         (0xF2)
#define DEV_CMD_DISABLE_SCANNING                 (0xF5)
#define DEV_CMD_ENABLE_SCANNING                  (0xF4)

#define DEV_RES_ACK                              (0xFA)
#define DEV_RES_BAT_COMPLETE                     (0xAA)

#define IS_READ_READY(status)                    (((status)&0x1) == 1)
#define IS_WRITE_READY(status)                   (((status)&0x2) == 0)

#define RW_TIMEOUT_ITERATION                     4000U

#define CONFIG_FIRST_PORT_INTERRUPT_ENABLE_BIT   (0U)
#define CONFIG_SECOND_PORT_INTERRUPT_ENABLE_BIT  (1U)
#define CONFIG_FIRST_PORT_CLOCK_DISABLE_BIT      (4U)
#define CONFIG_SECOND_PORT_CLOCK_DISABLE_BIT     (5U)
#define CONFIG_FIRST_PORT_TRANSLATION_ENABLE_BIT (6U)

bool ps2_port_1_available, ps2_port_2_available;

static UINT read_port (UINT port)
{
    // Wait for data to be in
    UINT i = 0, status = 0;
    inb (CMD_PORT, status);
    for (; !IS_READ_READY (status) && i < RW_TIMEOUT_ITERATION; i++) {
        inb (CMD_PORT, status);
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
    inb (CMD_PORT, status);
    for (; !IS_WRITE_READY (status) && i < RW_TIMEOUT_ITERATION; i++) {
        inb (CMD_PORT, status);
    }
    k_assert (i < RW_TIMEOUT_ITERATION, "PS2: Long wait");

    outb (port, data);
}

static void read_configuration_byte (char* s)
{
    write_port (CMD_PORT, CMD_READ_CONFIGURATION_BYTE);
    UINT config = read_port (DATA_PORT);
    kdebug_println ("%s: %x", s, config);
}

void ps2_init()
{
    FUNC_ENTRY();

    read_configuration_byte ("First");

    // Step 1: Disable both PS/2 ports
    // --------------------------------------------------------
    write_port (CMD_PORT, CMD_DISABLE_FIRST_PORT);
    write_port (CMD_PORT, CMD_DISABLE_SECOND_PORT);

    // Step 2: Flush the output (from device perspective) buffer
    // --------------------------------------------------------
    U8 discard;
    inb (DATA_PORT, discard);

    // Step 3: Test PS2 controller
    // --------------------------------------------------------
    write_port (CMD_PORT, CMD_TEST_CONTROLLER);
    UINT result = read_port (DATA_PORT);
    k_assert (result == RES_TEST_CONTROLLER_PASSED, "PS2: Self test failed");

    // Step 4: Determine if there are two channels
    // --------------------------------------------------------
    ps2_port_1_available = ps2_port_2_available = true;
    write_port (CMD_PORT, CMD_TEST_FIRST_PORT);
    result               = read_port (DATA_PORT);
    ps2_port_1_available = (result == RES_TEST_PORT_PASSED);

    write_port (CMD_PORT, CMD_TEST_SECOND_PORT);
    result               = read_port (DATA_PORT);
    ps2_port_2_available = (result == RES_TEST_PORT_PASSED);

    k_assert (ps2_port_1_available || ps2_port_2_available, "PS2: No ports working.");

    // Step 5: Setup configuration byte
    // --------------------------------------------------------
    write_port (CMD_PORT, CMD_READ_CONFIGURATION_BYTE);
    UINT config = read_port (DATA_PORT);

    if (ps2_port_1_available) {
        config = BIT_SET (config, CONFIG_FIRST_PORT_INTERRUPT_ENABLE_BIT);
        config = BIT_CLEAR (config, CONFIG_FIRST_PORT_TRANSLATION_ENABLE_BIT);
    }

    if (ps2_port_1_available) {
        config = BIT_SET (config, CONFIG_SECOND_PORT_INTERRUPT_ENABLE_BIT);
    }

    write_port (CMD_PORT, CMD_WRITE_CONFIGURATION_BYTE);
    write_port (DATA_PORT, config);
    kdebug_println ("Config written: %x", config);

    read_configuration_byte ("step 4");

    // Step 6: Enable both PS/2 ports
    // --------------------------------------------------------
    write_port (CMD_PORT, CMD_ENABLE_FIRST_PORT);  // This will turn on the clock for port 1
    write_port (CMD_PORT, CMD_ENABLE_SECOND_PORT); // This will turn on the clock for port 2

    read_configuration_byte ("step 5");

    // Keyboard setup
    if (ps2_port_1_available) {
        // Identify device
        write_port (DATA_PORT, DEV_CMD_DISABLE_SCANNING);
        k_assert (read_port (DATA_PORT) == DEV_RES_ACK, "ACK not received");

        write_port (DATA_PORT, DEV_CMD_IDENTIFY);
        k_assert (read_port (DATA_PORT) == DEV_RES_ACK, "ACK not received");
        UINT a = read_port (DATA_PORT), b = read_port (DATA_PORT);
        kdebug_println ("Dev identity: %x %x", a, b);

        write_port (DATA_PORT, DEV_CMD_SET_TO_DEFAULT); // Set defaults
        k_assert (read_port (DATA_PORT) == DEV_RES_ACK, "ACK not received");

        write_port (DATA_PORT, DEV_CMD_ENABLE_SCANNING); // Enable scanning
        k_assert (read_port (DATA_PORT) == DEV_RES_ACK, "ACK not received");
    }

    // Mouse setup
    if (ps2_port_1_available) {
        // Identify device
        write_port (CMD_PORT, CMD_SEND_TO_SECOND_PORT);
        write_port (DATA_PORT, DEV_CMD_DISABLE_SCANNING);
        k_assert (read_port (DATA_PORT) == DEV_RES_ACK, "ACK not received");

        write_port (CMD_PORT, CMD_SEND_TO_SECOND_PORT);
        write_port (DATA_PORT, DEV_CMD_IDENTIFY);
        k_assert (read_port (DATA_PORT) == DEV_RES_ACK, "ACK not received");
        UINT a, b = 0;
        a = read_port (DATA_PORT);
        //b = read_port (DATA_PORT);
        kdebug_println ("Dev identity: %x %x", a, b);

        write_port (CMD_PORT, CMD_SEND_TO_SECOND_PORT);
        write_port (DATA_PORT, DEV_CMD_SET_TO_DEFAULT); // Set defaults
        k_assert (read_port (DATA_PORT) == DEV_RES_ACK, "ACK not received");

        write_port (CMD_PORT, CMD_SEND_TO_SECOND_PORT);
        write_port (DATA_PORT, DEV_CMD_ENABLE_SCANNING); // Enable scanning
        k_assert (read_port (DATA_PORT) == DEV_RES_ACK, "ACK not received");
    }

    read_configuration_byte ("final");
}
