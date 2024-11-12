/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86, PC Kernel - 8259 PIC dirver
 * -------------------------------------------------------------------------------------------------
 */

#include <stdbool.h>
#include <x86/io.h>
#include <types.h>
#include <kassert.h>
#include <utils.h>
#include <kdebug.h>
#include <drivers/x86/pc/8259_pic.h>

#define MASTER_CMD_PORT          0x20
#define MASTER_DATA_PORT         0x21
#define SLAVE_CMD_PORT           0xA0
#define SLAVE_DATA_PORT          0xA1

#define ICW1_ICW4_PRESENT        (1 << 0)
#define ICW1_ALWAYS_ONE          (1 << 4)

#define ICW3_IRQ_LINE_FOR_SLAVE  (2) // IRQ line 2 of the master is connected with the slave.
#define ICW4_8086_MICROPROCESSOR (1 << 0)
#define OCW2_EOI                 (1 << 5)
#define OCW3_READ_IRR            (1 << 3 | 1 << 1)
#define OCW3_READ_ISR            (1 << 3 | 1 << 1 | 1 << 0)

void pic_init (U8 master_vector_start, U8 slave_vector_start)
{
    FUNC_ENTRY ("Master interrupt vector: %x, Slave interrupt vector: %x", master_vector_start,
                slave_vector_start);

    k_assert (IS_ALIGNED (master_vector_start, 8), "Vector mask wrong alignment");
    k_assert (IS_ALIGNED (slave_vector_start, 8), "Vector mask wrong alignment");

    // ICW1
    U8 icw1 = (ICW1_ICW4_PRESENT | ICW1_ALWAYS_ONE);
    outb (MASTER_CMD_PORT, icw1);
    io_delay();
    outb (SLAVE_CMD_PORT, icw1);
    io_delay();

    // ICW2
    outb (MASTER_DATA_PORT, master_vector_start); // ICW2 - interrupt vector start (master)
    io_delay();
    outb (SLAVE_DATA_PORT, slave_vector_start); // ICW2 - interrupt vector start (slave)
    io_delay();

    // ICW3
    outb (MASTER_DATA_PORT, (1 << ICW3_IRQ_LINE_FOR_SLAVE)); // IR line 2 has a slave.
    io_delay();
    outb (SLAVE_DATA_PORT, ICW3_IRQ_LINE_FOR_SLAVE); // Slave identity
    io_delay();

    // ICW4
    outb (MASTER_DATA_PORT, ICW4_8086_MICROPROCESSOR);
    io_delay();
    outb (SLAVE_DATA_PORT, ICW4_8086_MICROPROCESSOR);
    io_delay();

    // Mask/Disable all IRQs expect IRQ2 in master.
    // IRQ2 connects to slave PIC.
    // OCW1
    outb (MASTER_DATA_PORT, 0xFB);
    io_delay();
    outb (SLAVE_DATA_PORT, 0xFF);
    io_delay();
}

void pic_enable_disable_irq (PIC_IRQ irq, bool enable)
{
    FUNC_ENTRY ("IRQ: %x, Enable: %u", irq, enable);

    U8 port = MASTER_DATA_PORT;
    if (irq >= 8) {
        port = SLAVE_DATA_PORT;
        irq -= 8; // IRQs start from 0 for both Master and Slave PIC.
    }

    k_assert (irq >= 0 && irq <= 8, "Invalid IRQ number");

    INT value = 0;
    inb (port, value); // Read Mask register of the selected PIC

    if (enable) {
        value &= ~(1 << irq); // Enable: Clear the mask for the IRQ.
    } else {
        value |= (1 << irq); // Disable: Set the mask for the IRQ.
    }

    // Write OCW1
    outb (port, value);
}

UINT pic_read_IRR_ISR (bool readISR)
{
    FUNC_ENTRY();

    U8 ocw3 = (readISR) ? OCW3_READ_ISR : OCW3_READ_IRR;

    outb (MASTER_CMD_PORT, ocw3);
    outb (SLAVE_CMD_PORT, ocw3);

    UINT master, slave;
    inb (MASTER_CMD_PORT, master);
    inb (SLAVE_CMD_PORT, slave);

    return (slave << 8U) | master;
}

void pic_send_eoi (PIC_IRQ irq)
{
    k_assert (irq >= 0 && irq <= 16, "Invalid IRQ number");

    if (irq >= 8) {
        outb (SLAVE_CMD_PORT, 0x20);
    }
    outb (MASTER_CMD_PORT, 0x20);
}
