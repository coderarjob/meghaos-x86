/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel 
*
* This is a test kernel to demostrate that the 'boot1' loader can actually load
* the kernel and jump to it.
* On successful execution, 'Kernel ready..' will be displayed on the top of the
* screen.
* ---------------------------------------------------------------------------
*
* Dated: 20th September 2020
*/

#include <kernel.h>
#include <stdarg.h>

struct tss {
    u32 prevtask;
    u32 esp0;
    u32 ss0;
    u32 esp1;
    u32 ss1;
    u32 esp2;
    u32 ss2;
    u32 cr3;
    u32 eip;
    u32 eflags;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u32 es;
    u32 cs;
    u32 ss;
    u32 ds;
    u32 fs;
    u32 gs;
    u32 ldt_seg;
    u16 trap;
    u16 iomap_base;
} __attribute__((packed));

#define IOMAP_SIZE 0x100        // Covers VGA ports and normal IO ports.

static struct tss tss_entry = {0};
static u8 *iopers = (u8 *)&tss_entry + sizeof(struct tss);

void __jump_to_usermode();

__attribute__((noreturn)) 
void __main()
{
    kdisp_init();
    printk(PK_ONSCREEN,"\r\nKernel starting..");
    
    // Usermode code segment
    kgdt_edit(GDT_MIN_INDEX, 0, 0xFFFFF, 0xFA, 0xD);
    // Usermode data segment
    kgdt_edit(GDT_MIN_INDEX+1, 0, 0xFFFFF, 0xF2, 0xD);

    // Set all ones to iomap
    memset(iopers, 0xFF, IOMAP_SIZE-1);
    iopers[IOMAP_SIZE-1] = 0xFF;        // The last byte must be all ones.
    iopers[103] &=0xef;                     // To give 0x64 port permissions.

    // TSS Segment
    kgdt_edit(GDT_MIN_INDEX+2, (u32)&tss_entry, 
            IOMAP_SIZE-1, 0xE9, 0x9);
    //kgdt_edit(GDT_MIN_INDEX+2, (u32)&tss_entry, 
     //       sizeof(struct tss) -1, 0xE9, 0x9);
    kgdt_write();

    // Setup defaults to TSS, so that we can return to kernel mode.
    tss_entry.ss0 = 0x10;
    tss_entry.esp0 = 0x27FFF;

    // IOMAP area determines the io port permissions when CPL > IOPL.
    // If the corresponding bit is set, access to that port is denied.
    // iomap area starts at an offset from the start of tss. 
    // In ourcase it starts right at the end of tss_entry.
    tss_entry.iomap_base = sizeof(struct tss);  

    u16 tss_seg_selector = ((GDT_MIN_INDEX + 2) << 3| 3);
    __asm__ volatile ("ltr %0;": /* no output */ : "m" (tss_seg_selector));

    // Jump to user mode
    __jump_to_usermode();

    while(1);
}

void usermode_main()
{
    u8 v;
    //__asm__ volatile ("in %%al, 0x64;":"=a"(v):);
    inb(0x33c,v);
    while(1);
}
