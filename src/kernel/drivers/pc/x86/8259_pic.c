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

#define MASTER_CMD_PORT          0x20
#define MASTER_DATA_PORT         0x21
#define SLAVE_CMD_PORT           0xA0
#define SLAVE_DATA_PORT          0xA1

#define ICW1_ICW4_PRESENT        (1 << 0)
#define ICW1_ALWAYS_ONE          (1 << 4)

#define ICW3_IRQ_LINE_FOR_SLAVE  (2) // IRQ line 2 of the master is connected with the slave.

#define ICW4_8086_MICROPROCESSOR (1 << 0)

#define OCW2_EOI                 (1 << 5)

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
}

void pic_set_irq (INT irq, bool enable)
{
    FUNC_ENTRY ("IRQ: %x, Enable: %u", irq, enable);
}

void pic_get_irq (INT irq)
{
    FUNC_ENTRY ("IRQ: %x", irq);
}

void pic_send_eoi (U32 irq)
{
    if (irq >= 8) {
        outb (SLAVE_CMD_PORT, 0x20);
    }
    outb (MASTER_CMD_PORT, 0x20);
}
