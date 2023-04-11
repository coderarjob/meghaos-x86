/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - x86 Interrupt handlers - Interrupt and exception handlers
* --------------------------------------------------------------------------------------------------
*/

#ifndef INTERRUPT_H_X86
#define INTERRUPT_H_X86

#include <types.h>
#include <utils.h>
#include <moslimits.h>
#include <buildcheck.h>

typedef struct InterruptFrame  {
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t sp;
    uint32_t ss;
} __attribute__((packed)) InterruptFrame;

#define INTERRUPT_HANDLER(fn)                                           \
    void fn ## _handler (InterruptFrame *);                             \
    __asm__ (                                                           \
            ".globl " #fn "_asm_handler\n"                              \
            #fn "_asm_handler:\n"                                       \
            "pushad\n"                                                  \
            "push ds\n"                                                 \
            "push es\n"                                                 \
            "push fs\n"                                                 \
            "push gs\n"                                                 \
            "lea ebp, [esp + " STR(INTERRUPT_FRAME_SIZE_BYTES) "]\n"    \
            "push ebp\n"                                                \
            "call " #fn "_handler\n"                                    \
            "add esp, 4\n"                                              \
            "pop gs\n"                                                  \
            "pop fs\n"                                                  \
            "pop es\n"                                                  \
            "pop ds\n"                                                  \
            "popad\n"                                                   \
            "iret\n");

#define EXCEPTION_HANDLER INTERRUPT_HANDLER

#define EXCEPTION_WITH_CODE_HANDLER(fn)                                 \
    void fn ## _handler (InterruptFrame*, U32);                         \
    __asm__ (                                                           \
            ".globl " #fn "_asm_handler\n"                              \
            #fn "_asm_handler:\n"                                       \
            "pushad\n"                                                  \
            "push ds\n"                                                 \
            "push es\n"                                                 \
            "push fs\n"                                                 \
            "push gs\n"                                                 \
            "lea ebp, [esp + " STR(INTERRUPT_FRAME_SIZE_BYTES) "]\n"    \
            "mov eax, [ebp]\n"                                          \
            "lea ebx, [ebp + 4]\n"                                      \
            "push eax\n"                                                \
            "push ebx\n"                                                \
            "call " #fn "_handler\n"                                    \
            "add esp, 8\n"                                              \
            "pop gs\n"                                                  \
            "pop fs\n"                                                  \
            "pop es\n"                                                  \
            "pop ds\n"                                                  \
            "popad\n"                                                   \
            "iret\n");

void sys_dummy_asm_handler ();
void page_fault_asm_handler ();
void page_fault_asm_handler ();
void general_protection_fault_asm_handler ();
void div_zero_asm_handler ();

#endif // INTERRUPT_H_x86
