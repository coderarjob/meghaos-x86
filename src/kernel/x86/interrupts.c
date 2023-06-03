#include <types.h>
#include <x86/interrupt.h>
#include <disp.h>
#include <panic.h>
#include <kdebug.h>

INTERRUPT_HANDLER(sys_dummy)
void sys_dummy_handler (InterruptFrame *frame)
{
    (void)frame;
    kearly_printf ("\r\nInside sys_dummy routine..");
    __asm__ volatile ("mov ebx, %0"::"i"(0xffa1):"ebx");
    kbochs_breakpoint();
    //outb (0x80,4);
}

EXCEPTION_WITH_CODE_HANDLER(page_fault)
__attribute__ ((noreturn))
void page_fault_handler (InterruptFrame *frame, UINT errorcode)
{
    (void)frame;
    register INT fault_addr;
    __asm__ volatile ("mov %0, %%cr2":"=r"(fault_addr));

    k_panic ("Page fault when accessing address 0x%x (error: 0x%x)",
            fault_addr,errorcode);
}

EXCEPTION_WITH_CODE_HANDLER(general_protection_fault)
__attribute__ ((noreturn))
void general_protection_fault_handler (InterruptFrame *frame, UINT errorcode)
{
    (void)frame;
    (void)errorcode;

    k_panic ("%s","General protection fault.");
}

EXCEPTION_HANDLER(div_zero)
__attribute__ ((noreturn))
void div_zero_handler (InterruptFrame *frame)
{
    (void)frame;
    k_panic ("%s","Error: Division by zero");
}
