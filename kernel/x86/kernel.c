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
    u32 iomap_base;
} __attribute__((packed));

static struct tss tss_entry;

__attribute__((noreturn)) 
void __main()
{
    kdisp_init();
    printk(PK_ONSCREEN,"\r\nKernel starting..");
   
    // Usermode code segment
    kgdt_edit(GDT_MIN_INDEX, 0, 0xFFFFF, 0xFA, 0xD);
    // Usermode data segment
    kgdt_edit(GDT_MIN_INDEX+1, 0, 0xFFFFF, 0xF2, 0xD);

    // TSS Segment
    kgdt_edit(GDT_MIN_INDEX+2, (u32)&tss_entry, 
            sizeof(struct tss) -1, 0xE9, 0x9);

    kgdt_write();

    while(1);
}

void jump_to_usermode() {
         
}
