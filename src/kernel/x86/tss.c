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

#include <types.h>
#include <kstdlib.h>
#include <utils.h>
#include <x86/gdt.h>
#include <x86/memloc.h>
#include <kdebug.h>

#define IOMAP_SIZE 0x100        // Covers VGA ports and normal IO ports.

struct tss {
    U32 prevtask;
    U32 esp0;
    U32 ss0;
    U32 esp1;
    U32 ss1;
    U32 esp2;
    U32 ss2;
    U32 cr3;
    U32 eip;
    U32 eflags;
    U32 eax;
    U32 ecx;
    U32 edx;
    U32 ebx;
    U32 esp;
    U32 ebp;
    U32 esi;
    U32 edi;
    U32 es;
    U32 cs;
    U32 ss;
    U32 ds;
    U32 fs;
    U32 gs;
    U32 ldt_seg;
    U16 trap;
    U16 iomap_base;
    U32 ssp;
    U8 iomap[IOMAP_SIZE];
} __attribute__ ((packed));

static struct tss tss_entry = {0};

/* Initializes the tss_entry structure, 
 * installs a tss segment in GDT and writes to the Task Register*/
void ktss_init ()
{
    FUNC_ENTRY();

    // Deny permissions to all IO ports by setting all the bits
    k_memset (tss_entry.iomap, 0xFF, IOMAP_SIZE - 1);
    tss_entry.iomap[29] &=0xFD;     // E9 Debug port
    tss_entry.iomap[122] &=0xcf;    // VGA 3D4 and 3D5 ports

    // IOMAP area determines the io port permissions when CPL > IOPL.
    // If the corresponding bit is set, access to that port is denied.
    // iomap area starts at an offset from the start of tss. 
    // In ourcase it starts right at the end of tss_entry.
    tss_entry.iomap_base = offsetOf (struct tss,iomap); 
    // Setup defaults to TSS, so that we can return to kernel mode.
    // Setup a proper place for the kernel stack. This is the location the
    // ESP will have when returning to kernel mode from user mode. On a cross
    // privilate level INT instruction, the stack will have the 
    // user mode CS, EIP, EFLAGS, SS, ESP.
    tss_entry.ss0 = GDT_SELECTOR_KDATA;
    tss_entry.esp0 = INTEL_32_KSTACK_TOP;

    // Install a TSS Segment
    kgdt_edit (GDT_INDEX_KTSS,
            (U32)&tss_entry,            // Base in the Physical Linear space.
            sizeof (struct tss) -1,      // Size of the Segemnt = sizeof (tss)-1
            0xE9, 0x1);                 // DPL = 3, Scaling is not required.
    kgdt_write ();

    // Write to TS register
    // The Busy word will get set to indicate that it is the task that is
    // running at the point.
    U16 tss_seg_selector = GDT_SELECTOR_KTSS;
    __asm__ volatile ("ltr %0;"
                      : /* no output */ 
                      : "m" (tss_seg_selector));
}
