/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - TSS structure
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

#define IOMAP_SIZE 0x100        // Covers VGA ports and normal IO ports.

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
    u8 iomap[IOMAP_SIZE];
} __attribute__((packed));

static struct tss tss_entry = {0};

/* Initializes the tss_entry structure, 
 * installs a tss segment in GDT and writes to the Task Register*/
void ktss_init()
{
    // Deny permissions to all IO ports by setting all the bits
    memset(tss_entry.iomap, 0xFF, IOMAP_SIZE-1);
    tss_entry.iomap[122] &=0xcf;    // To give VGA ports.
    // IOMAP area determines the io port permissions when CPL > IOPL.
    // If the corresponding bit is set, access to that port is denied.
    // iomap area starts at an offset from the start of tss. 
    // In ourcase it starts right at the end of tss_entry.
    tss_entry.iomap_base = OFFSET_OF(struct tss,iomap); 
    // Setup defaults to TSS, so that we can return to kernel mode.
    // Setup a proper place for the kernel stack. This is the location the
    // ESP will have when returning to kernel mode from user mode. On a cross
    // privilate level INT instruction, the stack will have the 
    // user mode CS, EIP, EFLAGS, SS, ESP.
    tss_entry.ss0 = GDT_SELECTOR_KDATA;
    tss_entry.esp0 = INTEL_32_KSTACK_TOP;

    // Install a TSS Segment
    kgdt_edit(GDT_INDEX_KTSS,
            (u32)&tss_entry,            // Base in the Physical Linear space.
            sizeof(struct tss) -1,      // Size of the Segemnt = sizeof(tss)-1
            0xE9, 0x1);                 // DPL = 3, Scaling is not required.
    kgdt_write();

    // Write to TS register
    // The Busy word will get set to indicate that it is the task that is
    // running at the point.
    u16 tss_seg_selector = GDT_SELECTOR_KTSS;
    __asm__ volatile ("ltr %0;"
                      : /* no output */ 
                      : "m" (tss_seg_selector));
}
