/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - x86 Interrupt handlers - Interrupt and exception handlers
*
* Note:
* Must be compiled with -mgeneral-regs-only GCC attribute. This makes GCC only general purpose
* registers and not use SSE, MMX etc registers which are not preserved by the interrupt and
* exception handlers.
* --------------------------------------------------------------------------------------------------
*/

#ifndef INTERRUPT_H_X86
#define INTERRUPT_H_X86

#include <types.h>

typedef struct InterruptFrame  {
    U32 ip;
    U32 cs;
    U32 flags;
    U32 sp;
    U32 ss;
} __attribute__((packed)) InterruptFrame;

// Global table which pointer to system call functions.
extern void *g_syscall_table[];

#define INTERRUPT_HANDLER(fn)                                           \
    void fn ## _handler (InterruptFrame *);                             \
    __asm__ (                                                           \
            ".globl " #fn "_asm_handler\n"                              \
            #fn "_asm_handler:\n"                                       \
            "push ebp\n"                                                \
            "mov ebp, esp\n"                                            \
            "pushad\n"                                                  \
            "push ds\n"                                                 \
            "push es\n"                                                 \
            "push fs\n"                                                 \
            "push gs\n"                                                 \
            "lea eax, [ebp + 4]\n"                                      \
            "push eax\n"                                                \
            "call " #fn "_handler\n"                                    \
            "add esp, 4\n"                                              \
            "pop gs\n"                                                  \
            "pop fs\n"                                                  \
            "pop es\n"                                                  \
            "pop ds\n"                                                  \
            "popad\n"                                                   \
            "pop ebp\n"                                                 \
            "iret\n");

#define EXCEPTION_HANDLER INTERRUPT_HANDLER

#define EXCEPTION_WITH_CODE_HANDLER(fn)                                   \
    void fn##_handler (InterruptFrame*, U32);                             \
    __asm__(".globl " #fn "_asm_handler\n" #fn "_asm_handler:\n"          \
            "/////////////////////////\n"                                 \
            "// For call stack trace to work, EBP must be followed\n"     \
            "// by the return address in Stack. That is not true\n"       \
            "// here because of the 'error code' as part of the \n"       \
            "// interrupt frame as its last item.\n"                      \
            "// So we create a temporary space in the stack and\n"        \
            "// copy the return address from the interrupt frame.\n"      \
            "sub esp, 4\n"                                                \
            "push ebp\n"                                                  \
            "mov ebp, esp\n"                                              \
            "// Copy the return address from interrupt frame to the\n"    \
            "// storage allocated above.\n"                               \
            "push eax\n"                                                  \
            "mov eax, [ebp + 12]\n"                                       \
            "mov [ebp + 4], eax\n"                                        \
            "pop eax\n"                                                   \
            "/////////////////////////\n"                                 \
            "pushad\n"                                                    \
            "push ds\n"                                                   \
            "push es\n"                                                   \
            "push fs\n"                                                   \
            "push gs\n"                                                   \
            "mov eax, [ebp + 8]\n"                                        \
            "lea ebx, [ebp + 12]\n"                                       \
            "push eax\n"                                                  \
            "push ebx\n"                                                  \
            "call " #fn "_handler\n"                                      \
            "add esp, 8\n"                                                \
            "pop gs\n"                                                    \
            "pop fs\n"                                                    \
            "pop es\n"                                                    \
            "pop ds\n"                                                    \
            "popad\n"                                                     \
            "pop ebp\n"                                                   \
            "add esp, 4\n"                                                \
            "// IRET will not remove the error code, so need to remove\n" \
            "// before IRET.\n"                                           \
            "add esp, 4\n"                                                \
            "iret\n");

void sys_dummy_asm_handler ();
void page_fault_asm_handler ();
void page_fault_asm_handler ();
void double_fault_asm_handler();
void general_protection_fault_asm_handler ();
void div_zero_asm_handler ();
void syscall_asm_despatcher ();

#endif // INTERRUPT_H_x86
