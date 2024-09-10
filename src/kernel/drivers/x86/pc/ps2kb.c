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

bool ps2kb_init()
{
    FUNC_ENTRY();

    // Disable Keyboard IRQ before the initialization is complete
    pic_enable_disable_irq (PIC_IRQ_KEYBOARD, false);

    // Check if PS2 port1 is Keyboard.
    PS2DeviceType dtype = ps2_identify_device (&port1);
    if (dtype != PS2_DEVICE_TYPE_KEYBOARD) {
        RETURN_ERROR (ERR_DEVICE_INIT_FAILED, false);
    }

    // Set defaults
    if (!ps2_write_device_cmd (&port1, true, PS2_DEV_CMD_SET_TO_DEFAULT)) {
        RETURN_ERROR (ERR_DEVICE_INIT_FAILED, false);
    }

    // Interrupt when keys are pressed
    if (!ps2_write_device_cmd (&port1, true, PS2_DEV_CMD_ENABLE_SCANNING)) {
        RETURN_ERROR (ERR_DEVICE_INIT_FAILED, false);
    }

    // Setup PS2 configuration to enable interrupts from port 1 and disable translation
    ps2_setup_port_configuration (&port1, true, false);

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
    U8 scancode = (U8)ps2_read_no_wait(PS2_DATA_PORT);
    kearly_println ("Keyboard ISR: Scancode: %x", scancode);
    pic_send_eoi (PIC_IRQ_KEYBOARD);
}
