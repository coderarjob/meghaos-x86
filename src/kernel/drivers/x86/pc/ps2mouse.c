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

bool ps2mouse_init()
{
    FUNC_ENTRY();

    // Disable Mouse IRQ before the initialization is complete
    pic_enable_disable_irq (X86_PC_IRQ_PS2_MOUSE, false);

    // Check if PS2 port2 is Mouse
    PS2DeviceType dtype = ps2_identify_device (&port2);
    if (dtype != PS2_DEVICE_TYPE_MOUSE) {
        RETURN_ERROR (ERR_DEVICE_INIT_FAILED, false);
    }

    // Set defaults
    if (!ps2_write_device_cmd (&port2, true, PS2_DEV_CMD_SET_TO_DEFAULT)) {
        RETURN_ERROR (ERR_DEVICE_INIT_FAILED, false);
    }

    // Interrupt when keys are pressed
    if (!ps2_write_device_cmd (&port2, true, PS2_DEV_CMD_ENABLE_SCANNING)) {
        RETURN_ERROR (ERR_DEVICE_INIT_FAILED, false);
    }

    // Setup PS2 configuration to enable interrupts from port 2
    ps2_setup_port_configuration (&port2, true, false);

    // Add handlers for keyboard interrupts
    kidt_edit (0x2C, mouse_interrupt_asm_handler, GDT_SELECTOR_KCODE,
               IDT_DES_TYPE_32_INTERRUPT_GATE, 0);

    // Enable Mouse IRQ
    pic_enable_disable_irq (X86_PC_IRQ_PS2_MOUSE, true);

    return true;
}

INTERRUPT_HANDLER (mouse_interrupt)
void mouse_interrupt_handler (InterruptFrame* frame)
{
    (void)frame;
    U8 data1, data2, data3 = 0;
    data1 = (U8)ps2_read_data_no_wait();
    data2 = (U8)ps2_read_data_no_wait();
    kearly_println ("Mouse handler: %x, %x, %x", data1, data2, data3);
    pic_send_eoi (X86_PC_IRQ_PS2_MOUSE);
}
