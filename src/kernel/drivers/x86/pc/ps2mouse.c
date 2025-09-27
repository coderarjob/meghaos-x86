/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86, PC Kernel - PS/2 Mouse Driver
 * -------------------------------------------------------------------------------------------------
 */

#include <drivers/x86/pc/8042_ps2.h>
#include <kerror.h>
#include <drivers/x86/pc/8259_pic.h>
#include <x86/idt.h>
#include <x86/interrupt.h>
#include <x86/gdt.h>
#include <utils.h>
#include <drivers/x86/pc/ps2_devices.h>

typedef struct MousePacket {
    INT byte_index;
    U8 b0;
    U8 b1;
    U8 b2;
} MousePacket;

typedef struct MouseStatus {
    U8 id;
    U8 status;
    U8 resolution;
    U8 sample_rate;
} MouseStatus;

void ps2_mouse_interrupt_asm_handler(void);
static bool ismouse(void);
static MouseStatus get_mouse_state(void);

static MousePositionData mouse_position  = { 0 };
static MouseStatus mouse_status_original = { 0 };

static bool ismouse(void)
{
    INT type = 0;
    if ((type = ps2_identify_device (PS2_SECOND_DEVICE)) < 0) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    switch (type) {
    case 0x0000:
    case 0x0300:
    case 0x0400:
        // Valid mouse identity. Continue
        break;
    default:
        RETURN_ERROR (ERR_DEVICE_INIT_FAILED, false);
    }
    return true;
}

static MouseStatus get_mouse_state(void)
{
    MouseStatus status = { 0 };

    ps2_write_device_data_wait_ack (PS2_SECOND_DEVICE, PS2_MOUSE_CMD_GET_ID);
    ps2_wait_read (PS2_DATA_PORT, &status.id);

    ps2_write_device_data_wait_ack (PS2_SECOND_DEVICE, PS2_MOUSE_CMD_STATUS_REQ);
    ps2_wait_read (PS2_DATA_PORT, &status.status);
    ps2_wait_read (PS2_DATA_PORT, &status.resolution);
    ps2_wait_read (PS2_DATA_PORT, &status.sample_rate);

    return status;
}

bool ps2_mouse_init(void)
{
    FUNC_ENTRY();

    // Disable Mouse IRQ before the initialization is complete
    pic_enable_disable_irq (PIC_IRQ_PS2_MOUSE, false);

    // Check if PS2 port2 is Mouse
    if (!ismouse()) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    // Set defaults
    if (!ps2_write_device_data_wait_ack (PS2_SECOND_DEVICE, PS2_DEV_CMD_SET_TO_DEFAULT)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    // Set sample rate
    if (!ps2_write_device_data_wait_ack (PS2_SECOND_DEVICE, PS2_MOUSE_CMD_SET_SAMPLE_RATE)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }
    if (!ps2_write_device_data_wait_ack (PS2_SECOND_DEVICE, CONFIG_PS2_MOUSE_SAMPLE_RATE)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    // Enable scanning by the device
    if (!ps2_write_device_data_wait_ack (PS2_SECOND_DEVICE, PS2_DEV_CMD_ENABLE_SCANNING)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    // Check if mouse status is as per settings set previously
    mouse_status_original = get_mouse_state();

    if (mouse_status_original.sample_rate != CONFIG_PS2_MOUSE_SAMPLE_RATE ||
        !BIT_ISSET (mouse_status_original.status,
                    PS2_MOUSE_STATE_BYTE_0_DATA_REPORTING_ENABLED_MASK) ||
        BIT_ISSET (mouse_status_original.status, PS2_MOUSE_STATE_BYTE_0_REMOTE_MODE_ENABLED)) {
        // Device state is not as per expected.
        RETURN_ERROR (ERR_DEVICE_INIT_FAILED, false);
    }

    // Setup PS2 configuration to enable interrupts from port 2
    ps2_configuration (PS2_CONFIG_SECOND_PORT_INTERRUPT_ENABLE, 0, NULL);

    // Add handlers for keyboard interrupts
    kidt_edit (0x2C, ps2_mouse_interrupt_asm_handler, GDT_SELECTOR_KCODE,
               IDT_DES_TYPE_32_INTERRUPT_GATE, 0);

    // Enable Mouse IRQ
    pic_enable_disable_irq (PIC_IRQ_PS2_MOUSE, true);

    return true;
}

MousePositionData ps2_mouse_get_packet(void)
{
    return mouse_position;
}

INTERRUPT_HANDLER (ps2_mouse_interrupt)
void ps2_mouse_interrupt_handler (InterruptFrame* frame)
{
    (void)frame;

    static MousePacket mpacket = { 0 };

    U8 data;
    if (ps2_wait_read (PS2_DATA_PORT, &data)) {
        switch (mpacket.byte_index) {
        case 0:
            mpacket.b0 = data;
            break;
        case 1:
            mpacket.b1 = data;
            break;
        case 2:
            mpacket.b2 = data;

            INFO ("Mouse packet: %x %x %x", mpacket.b0, mpacket.b1, mpacket.b2);

            // Discard whole packet is overflow is set
            if (!BIT_ISSET (mpacket.b0, PS2_MOUSE_PACKET_BYTE0_X_OVERFLOW_BTN_MASK) &&
                !BIT_ISSET (mpacket.b0, PS2_MOUSE_PACKET_BYTE0_Y_OVERFLOW_BTN_MASK)) {
                mouse_position.x += mpacket.b1 - ((mpacket.b0 << 4) & 0x100);
                mouse_position.y += mpacket.b2 - ((mpacket.b0 << 3) & 0x100);
                mouse_position.left_button   = BIT_ISSET (mpacket.b0,
                                                          PS2_MOUSE_PACKET_BYTE0_LEFT_BTN_MASK);
                mouse_position.right_button  = BIT_ISSET (mpacket.b0,
                                                          PS2_MOUSE_PACKET_BYTE0_RIGHT_BTN_MASK);
                mouse_position.middle_button = BIT_ISSET (mpacket.b0,
                                                          PS2_MOUSE_PACKET_BYTE0_MID_BTN_MASK);
            } else {
                WARN ("PS2: Mouse overflow detected.");
            }
            break;
        default:
            FATAL_BUG();
        }
        mpacket.byte_index = (mpacket.byte_index + 1) % 3;
    }

    pic_send_eoi (PIC_IRQ_PS2_MOUSE);
}
