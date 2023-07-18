#include <stdarg.h>
#include <types.h>
#include <x86/interrupt.h>
#include <disp.h>
#include <panic.h>
#include <kdebug.h>
#include <assert.h>
#include <moslimits.h>

typedef struct GPFError
{
    U8 ext    :1;
    U8 table  :2;
    U16 index :13;
} __attribute__((packed)) GenProcFaultError;

typedef struct PageFaultError
{
    U8 Present                :1;
    U8 WriteFault             :1;
    U8 UserMode               :1;
    U8 ReserveViolation       :1;
    U16 InstrucionFetchFault  :1;
    U8 ProtectionkeyViolation :1;
    U8 ShadowStackAccessFault :1;
    U8 reserved_1             :1;
    U8 reserved_2             :7;
    U8 SGXViolation           :1;
    U16 reserved_3;
} __attribute__((packed)) PageFaultError;

static void s_appendStackFrame(InterruptFrame *frame, char *buffer, INT size);
static void s_callPanic(InterruptFrame *frame, char *fmt, ...);

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
    register INT fault_addr;
    __asm__ volatile ("mov %0, %%cr2":"=r"(fault_addr));

    PageFaultError *err = (PageFaultError*) &errorcode;
    s_callPanic(frame, "Page fault when accessing address 0x%x (error: 0x%x)"
                       "\r\n\r\n- P: %x\r\n- Write: %x\r\n- UserM: %x\r\n- ResV: %x"
                       "\r\n- InsF: %x\r\n- PKV: %x\r\n- SSA: %x\r\n- SGX: %x",
                       fault_addr, errorcode,
                       err->Present, err->WriteFault, err->UserMode, err->ReserveViolation,
                       err->InstrucionFetchFault, err->ProtectionkeyViolation,
                       err->ShadowStackAccessFault, err->SGXViolation);
}

EXCEPTION_WITH_CODE_HANDLER(general_protection_fault)
__attribute__ ((noreturn))
void general_protection_fault_handler (InterruptFrame *frame, UINT errorcode)
{
    GenProcFaultError *err = (GenProcFaultError*) &errorcode;
    s_callPanic(frame, "General Protection Fault.\r\n- Ext: %x\r\n- Table: %x\r\n- Index: %x",
                            err->ext,
                            err->table,
                            err->index);

}

EXCEPTION_HANDLER(div_zero)
__attribute__ ((noreturn))
void div_zero_handler (InterruptFrame *frame)
{
    (void)frame;
    s_callPanic(frame,"%s","Division by zero.");
}

__attribute__ ((noreturn))
static void s_callPanic(InterruptFrame *frame, char *fmt, ...)
{
    char buffer[MAX_PRINTABLE_STRING_LENGTH] = {0};
    INT size = sizeof(buffer);

    va_list l;
    va_start(l, fmt);

    INT len = kearly_vsnprintf (buffer, (size_t) size, fmt, l);
    va_end(l);

    k_assert(size > len, "Buffer size too small.");

    s_appendStackFrame(frame, buffer + len, size - len);

    k_panic("%s", buffer);
}

static void s_appendStackFrame(InterruptFrame *frame, char *buffer, INT size)
{
    k_assert(size > 0, "Buffer size too small.");

    size -= kearly_snprintf(buffer, (size_t) size,
            "\r\n\r\nInterrupt Frame:"
             "\r\n- EIP: 0x%x\r\n- CS: 0x%x\r\n- EFLAGS: 0x%x\r\n- ESP: 0x%x\r\n- SS: 0x%x",
             frame->ip, frame->cs, frame->flags, frame->sp, frame->ss);

    k_assert(size > 0, "Buffer size too small.");
}
