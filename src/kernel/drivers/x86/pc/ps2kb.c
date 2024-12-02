/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86, PC Kernel - PS/2 Keyboard Driver
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

void kb_interrupt_asm_handler();

static bool iskeyboard()
{
    INT type = 0;
    if ((type = ps2_identify_device (PS2_FIRST_DEVICE)) < 0) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    switch (type) {
    case 0x83AB:
    case 0x41AB:
    case 0xC1AB:
    case 0x84AB:
    case 0x54AB:
        // Valid keyboard identity. Continue
        break;
    default:
        RETURN_ERROR (ERR_DEVICE_INIT_FAILED, false);
    }
    return true;
}

bool ps2_kb_init()
{
    FUNC_ENTRY();

    // Disable Keyboard IRQ before the initialization is complete
    pic_enable_disable_irq (PIC_IRQ_KEYBOARD, false);

    // Check if PS2 port1 is Keyboard.
    if (!iskeyboard()) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    // Set defaults
    if (!ps2_write_device_data_wait_ack (PS2_FIRST_DEVICE, PS2_DEV_CMD_SET_TO_DEFAULT)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    // Interrupt when keys are pressed
    if (!ps2_write_device_data_wait_ack (PS2_FIRST_DEVICE, PS2_DEV_CMD_ENABLE_SCANNING)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    // Setup PS2 configuration to enable interrupts from port 1 and disable translation
    ps2_configuration (PS2_CONFIG_FIRST_PORT_INTERRUPT_ENABLE |
                           PS2_CONFIG_FIRST_PORT_TRANSLATION_ENABLE,
                       0, NULL);

    // Add handlers for keyboard interrupts
    kidt_edit (0x21, kb_interrupt_asm_handler, GDT_SELECTOR_KCODE, IDT_DES_TYPE_32_INTERRUPT_GATE,
               0);

    // Enable Keyboard IRQ
    pic_enable_disable_irq (PIC_IRQ_KEYBOARD, true);

    return true;
}

INTERRUPT_HANDLER (kb_interrupt)
void kb_interrupt_handler (InterruptFrame* frame)
{
    (void)frame;

#if defined(DEBUG)
    U8 scancode = (U8)ps2_no_wait_read (PS2_DATA_PORT);
    kearly_println ("Keyboard ISR: Scancode: %x", scancode);
#endif
    pic_send_eoi (PIC_IRQ_KEYBOARD);
}
