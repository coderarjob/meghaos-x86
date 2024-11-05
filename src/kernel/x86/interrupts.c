/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - x86 kernel - HW and SW interrupt handlers
 * ---------------------------------------------------------------------------
 */
#include <stdarg.h>
#include <types.h>
#include <x86/interrupt.h>
#include <utils.h>
#include <disp.h>
#include <panic.h>
#include <kdebug.h>
#include <kassert.h>
#include <moslimits.h>
#include <vmm.h>
#include <kernel.h>
#include <process.h>
#if MARCH == pc
    #include <drivers/x86/pc/8259_pic.h>
#endif

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

#if MARCH == pc
INTERRUPT_HANDLER (timer_interrupt)
void timer_interrupt_handler (InterruptFrame* frame)
{
    (void)frame;
    // As we are simply incrementing the tick_count every time the timer expires.
    k_staticAssert (CONFIG_TICK_PERIOD_MICROSEC == CONFIG_INTERRUPT_CLOCK_TP_MICROSEC);

    g_kstate.tick_count++;
    keventmanager_invoke();

    pic_send_eoi (PIC_IRQ_TIMER);
}
#endif

INTERRUPT_HANDLER(sys_dummy)
void sys_dummy_handler (InterruptFrame *frame)
{
    (void)frame;
    kearly_println ("Inside sys_dummy routine..");
    __asm__ volatile ("mov ebx, %0"::"i"(0xffa1):"ebx");
    kbochs_breakpoint();
    //outb (0x80,4);
}

EXCEPTION_WITH_CODE_HANDLER(double_fault)
__attribute__ ((noreturn))
void double_fault_handler (InterruptFrame *frame, UINT errorcode)
{
    (void)frame;        // Frame is not guaranteed to be valid
    k_panic("Double fault - Fatal error (Error code: %x)", errorcode);
    NORETURN();
}

EXCEPTION_WITH_CODE_HANDLER(page_fault)
void page_fault_handler (InterruptFrame *frame, UINT errorcode)
{
    FUNC_ENTRY("frame: %px, error code: %x", frame, errorcode);

    PageFaultError *err = (PageFaultError*) &errorcode;
    register PTR fault_addr;
    __asm__ volatile ("mov %0, cr2":"=r"(fault_addr));
    INFO("Page fault: Address: %px", fault_addr);

    // Page faults can occur for various reasons, it could be permissions or if page frame is not
    // present. Commit a physical page only it there is not one already allocated.
    VMemoryManager* context = kvmm_checkbounds (g_kstate.context, fault_addr)
                                  ? g_kstate.context
                                  : kprocess_getCurrentContext();

    if (err->Present == false && kvmm_commitPage (context, fault_addr)) {
        return; // then retry
    }

    s_callPanic(frame, "Page fault when accessing address %x (error: %x)"
                       "\n\n- P: %x\n- Write: %x\n- UserM: %x\n- ResV: %x"
                       "\n- InsF: %x\n- PKV: %x\n- SSA: %x\n- SGX: %x",
                       fault_addr, errorcode,
                       err->Present, err->WriteFault, err->UserMode, err->ReserveViolation,
                       err->InstrucionFetchFault, err->ProtectionkeyViolation,
                       err->ShadowStackAccessFault, err->SGXViolation);
    //NORETURN();
}

EXCEPTION_WITH_CODE_HANDLER(general_protection_fault)
__attribute__ ((noreturn))
void general_protection_fault_handler (InterruptFrame *frame, UINT errorcode)
{
    GenProcFaultError *err = (GenProcFaultError*) &errorcode;
    s_callPanic(frame, "General Protection Fault.\n- Ext: %x\n- Table: %x\n- Index: %x",
                            err->ext,
                            err->table,
                            err->index);
    NORETURN();
}

EXCEPTION_HANDLER(div_zero)
__attribute__ ((noreturn))
void div_zero_handler (InterruptFrame *frame)
{
    s_callPanic(frame,"%s","Division by zero.");
    NORETURN();
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

    WARN_ON(size > len,  "Buffer size too small.");

    s_appendStackFrame(frame, buffer + len, size - len);

    k_panic("%s", buffer);
    NORETURN();
}

static void s_appendStackFrame(InterruptFrame *frame, char *buffer, INT size)
{
    size -= kearly_snprintf(buffer, (size_t) size,
            "\n\nInterrupt Frame:"
             "\n- EIP: %x\n- CS: %x\n- EFLAGS: %x\n- ESP: %x\n- SS: %x",
             frame->ip, frame->cs, frame->flags, frame->sp, frame->ss);
    (void)size;

    WARN_ON(size > 0,  "Buffer size too small.");
}
