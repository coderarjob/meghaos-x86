/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86, PC Kernel - PS/2 Mouse Driver
 * -------------------------------------------------------------------------------------------------
 */

#include <drivers/x86/pc/8042_ps2.h>
#include <kassert.h>
#include <stdbool.h>
#include <kerror.h>
#include <drivers/x86/pc/8259_pic.h>
#include <x86/idt.h>
#include <x86/interrupt.h>
#include <x86/gdt.h>
#include <x86/io.h>
#include <utils.h>

void mouse_interrupt_asm_handler();

static bool ismouse()
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

bool ps2mouse_init()
{
    FUNC_ENTRY();

    // Disable Mouse IRQ before the initialization is complete
    pic_enable_disable_irq (PIC_IRQ_PS2_MOUSE, false);

    // Check if PS2 port2 is Mouse
    if (!ismouse()) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    // Set defaults
    if (!ps2_write_device_cmd (PS2_SECOND_DEVICE, PS2_DEV_CMD_SET_TO_DEFAULT)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    // Interrupt when keys are pressed
    if (!ps2_write_device_cmd (PS2_SECOND_DEVICE, PS2_DEV_CMD_ENABLE_SCANNING)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    // Setup PS2 configuration to enable interrupts from port 2
    ps2_configuration (PS2_CONFIG_SECOND_PORT_INTERRUPT_ENABLE, 0, NULL);

    // Add handlers for keyboard interrupts
    kidt_edit (0x2C, mouse_interrupt_asm_handler, GDT_SELECTOR_KCODE,
               IDT_DES_TYPE_32_INTERRUPT_GATE, 0);

    // Enable Mouse IRQ
    pic_enable_disable_irq (PIC_IRQ_PS2_MOUSE, true);

    return true;
}

INTERRUPT_HANDLER (mouse_interrupt)
void mouse_interrupt_handler (InterruptFrame* frame)
{
    (void)frame;
#if defined(DEBUG) && defined(PORT_E9_ENABLED)
    U8 data1, data2, data3 = 0;
    data1 = (U8)ps2_no_wait_read (PS2_DATA_PORT);
    data2 = (U8)ps2_no_wait_read (PS2_DATA_PORT);
    INFO ("Mouse handler: %x, %x, %x", data1, data2, data3);
#endif
    pic_send_eoi (PIC_IRQ_PS2_MOUSE);
}
