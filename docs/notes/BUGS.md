# BUGS
 
[X] __func__ is giving garbage after paging.s was included.
    If paging.s is compiled with GAS then __func__ is working 
    in the kernel.c

    Solution:
    =========
    28th Dec 2020
    .rodata* section must be before .bss section the KERNEL.ELF file.

[X] %x, %d prints unsigned numbers, but _get_int_arg reads signed intgegers from
    stack. Assignment of signed integers to unsigned int variable, causes
    sign extension and this produces large output values.
    Example:
    printf("%x",0xC0000000) producing FFFFFFFFC0000000.

    Solution:
    =========
    5th Arg 2020
    Reading unsigned int variants in va_arg instead of just int, long etc.


[X] bug-no-boot-omega:
    Not able to boot on some native hardware. The screen goes black and after
    some cursor flickering, it settles someshere in the bottom row. Sometimes
    there is a Omega character with the cursor in the end.
    - It faults before passing control to the 2nd stage bootloader.
    - Setting the VGA mode (to 0x3 Text mode) has no effect on the bug.

    Solution:
    =========
    24th Dec 2021
    I never set the DS register. Aparently in some system the segment registers are not reset to 
    zero before passing control to the bootloader.

[ ] bug-printk-pagefault-wrong-formatstring
    If there is a wrong type character, followed by a valid one in the format string. Either wrong
    output is displayed, or we get a page fault.
    In the following diagram, the first type character '% ' is worng, when it reaches, '%s', it
    will wrongly read '12', and interpret it as a string pointer - Thus the page fault.

    Wrote a C program and passed the same arguments to printf. The program segfaults at the printf.
    However atleast gcc produces warnings when compiling. This is a tricky issue to fix, even for 
    libc.

    Reproduced by:
    kearly_printf ("% u, %s", 12, "Arjob");  -> Output: Page fault
    kearly_printf ("% u, %u", 12, "Arjob");  -> Output: '% u, 12'

[X] bug-address-colision-A
    Addresses at the .bss section, are just allocated, it contains no data. This means that .bss 
    section does not contribute to the size of the end binary. It can be that the .bss section is 
    1 MB, but the flat binary is only 10 bytes (containing only .data, .text).

    After the boo1 program loads the kernel image, it gets the size of the kernel file and
    updates the same to the boot info structure. This size determines where the boot1 loads the
    next ramdisks (if any) or where the kernel places the page directory and page table.

    As I am not considering the size of the .bss section, the kernel size is wrong and all the 
    subsequent operations that depend on kernel size is also wrong.

    Actual findings (this is Scenario 1, below):
    =============================================
    * kernel is loaded at     : 0xC0100000, binary file size is 5056 (0x1A2C) bytes.
    * Kernel file ends at     : 0xC0101A2C 
    * Page directory starts at: 0xC0102000, 4KiB aligned address after kernel file.
    * Page directory ends at  : 0xC0102FFF
    * Page table starts at    : 0xC0103000
    * Page table ends at      : 0xC0103FFF

    Now look at some of the variables in the .bss section of the kernel.
    c0102230 l     O .bss   00000001 row
    c0102231 l     O .bss   00000001 column
    c0102232 l     O .bss   00000001 crtc_flags

    Variable 'row', 'column', and 'crtc_flags' reside within the page directory!!
    The calculation for placement of page directory and thus page table is wrong, as it ignores the
    size of the .bss section of the kernel.

    Scenario 1:
    ==========
    The case where there are no ramdisks; just the kernel image.

    Kernel      |--------|-----------------|---------|------------------------------------------|
    addr range: |        | .text, .data    |   .bss  |                                          |
    ------------|--------|-----------------|------|--|-------|------------|---------------------|
    mos         |        |  kernel image   | Free | Page dir | Page table |                     |
    memory map  |--------|-----------------|------|-----------------------|---------------------|

    By ignoring the .bss section, we are storing page dir and page table at wrong locations,
    overlapping the addresses in the .bss section.

    Note: The 'Free' section results from the alignment of page directory to 4KB boundary.
    
    Scenario 2:
    ==========
    The case where there are ramdisks:

    Kernel      |--------|-----------------|---------|------------------------------------------|
    addr range: |        | .text, .data    |   .bss  |                                          |
    ------------|--------|-----------------|---------|-|-----------|----------------------------|
    mos         |        |  kernel image   | ramdisk 1 | ramdisk 2 |                            |
    memory map  |--------|-----------------|-----------|-----------|----------------------------|

    By ignoring the .bss section, we are placing ramdisk 1 in the same location, where the .bss is
    already assigned. 

    Solution:
    =========
    8th Jan 2022
    Problems in both the above two scenarios stem from the fact, that the bootloader does not know
    the true end of the kenrel. The .bss section size and its alignment, are both unknown, and 
    cannot be known from a flat binary.

    For the bootloader, the options are the following:
    1. Use some other binary format, which includes the 'start' and 'end' locations of each of the
       sections. Note that we will require the start and end locations, to account for any 
       alignment padding between sections.
    2. Remove the .bss section. That way, the size of the flat binary will match actual kernel 
       size. This means, to link all the .bss sections into .data section of the final elf kernel.

    Of these two solutions, I am going with the 2nd one. What is the rational?

    1. Simplicity. Implementing ELF or a.out loader in the bootloader in assembly will be time 
       consuming. I do not think I can handle such complexity and do not forget testing. 
       Further more, I want bootloader to be as simple as possible.
    2. Moving the .bss into the .data means, the end binary size will increase.
       * Size with .bss in the .bss section: 6668 Bytes
       * Size with .bss included in .data  : 7088 Bytes
       
       This is not too much of a concern, because, after the memory management is ready, almost 
       all the large allocations, will be allocated dynamically and thus no longer will use space 
       in the .bss section. Which means, the size of .bss section will decrease.

[ ] bug-address-colision-B
    At the time of running module programs (from the ramdisks), the OS, must know the size of its
    .bss section, so that it can allocate memory and setup paging properly.

    If the modules are raw flat binaries, there will not way for the OS to know!

    Solution:
    ==========
    1. Implement a simple binary file format, that stores the size of .bss section along with the 
       binary data.
    2. Implement either a.out or ELF within the kenrel. 

    No matter what the solution, it is clear, that the module files cannot be raw flat binary.

[-] Double allocation when accessing graphics backbuffer (kcompose_flush)
    * 8 bit color
    * kcompose_flush from user program

Not seen with a later commit 12aef3e

[-] Triple fault when multithread_demo_kernel_thread starts mpdemo in kernel mode.

Not seen with a later commit 12aef3e

[X] os_process_is_yield_requested not working properly if defined in applib/syscall.c
Cause: Compiler optimization.
Fix  : A 'volatile' keyword fixed the issue. Fixed in commit 7897a67

[X] os_process_is_yield_requested & os_yield not working in delay() function in applib/app.c.
Cause: Compiler optimization.
Fix  : A 'volatile' keyword fixed the issue. Fixed in commit 7897a67

[X] gui0 program hanging after running for sometime. NDEBUG, DEBUG_LEVEL=0 build in Qemu, bochs,
86box and laptop.

Reason:
A GP fault occurred because interrupt handler for vector 0x27 (or IRQ7) did not exit. IRQ7 (for
master PIC & IRQ15 for slave one) is called when a 'spurious' interrupt has occurred. Spurious
interrupts are triggered by PIC when the handshaking with the CPU has started but IR disappears
before it can complete. This happens if there's noise in the IR line or when software sends EOI
command.
Read: `https://wiki.osdev.org/8259_PIC#Spurious_IRQs`

In this case the spurious interrupt occurred when a EOI from timer interrupt handler lands at a
time when PIC has started another timer IRQ handshaking. 

In our case normally the timer interrupt does not take much time to complete, but when it needs to
update the video framebuffer it takes longer and can finish around the next timer IRQ. Note that
spurious interrupts may not occur immediately at the next timer IRQ but much later as it accumulates
delay with each interrupt handler call.

Solution:
Detect and handle the spurious interrupts. I am not sure if its possible to prevent spurious
interrupts due to wrong EOI timing from inside interrupt handlers. Anyways its a good idea to handle
spurious interrupts since they can also occur due to noise in the lines.

[ ] Duplicate defination of types if applib/types.h is included in `applib/*.c` files.
