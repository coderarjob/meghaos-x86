/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86, PC Kernel - 8259 PIC dirver headers
 * -------------------------------------------------------------------------------------------------
 */

#pragma once
#include <types.h>

typedef enum X86_PC_IRQ {
    X86_PC_IRQ_TIMER     = 0,
    X86_PC_IRQ_KEYBOARD  = 1,
    X86_PC_IRQ_PS2_MOUSE = 12,
} X86_PC_IRQ;

void pic_init (U8 master_vector_start, U8 slave_vector_start);
void pic_enable_disable_irq (X86_PC_IRQ irq, bool enable);
UINT pic_read_IRR_ISR (bool readISR);
void pic_send_eoi (X86_PC_IRQ irq);
