/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86, PC Kernel - 8259 PIC dirver headers
 * -------------------------------------------------------------------------------------------------
 */

#pragma once
#include <types.h>

typedef enum PC_IRQ {
    PIC_IRQ_TIMER     = 0,
    PIC_IRQ_KEYBOARD  = 1,
    PIC_IRQ_PS2_MOUSE = 12,
} PIC_IRQ;

void pic_init (U8 master_vector_start, U8 slave_vector_start);
void pic_enable_disable_irq (PIC_IRQ irq, bool enable);
void pic_read_IRR_ISR (bool readISR, UINT *master, UINT *slave);
void pic_send_eoi (PIC_IRQ irq);
