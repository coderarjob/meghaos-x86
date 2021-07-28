ENTRY (__kernel_entry)
SECTIONS
{
    . = 0x28000;                 /* Kernel is loaded at 0x2800:0x0000 */
    KERNEL_START = .;
    .prepage.bss :
    {
        *.o (.prepage.bss);
    }
    .prepage.text :
    {
        *.o (.prepage.text);
    }
    PREPAGE_END = .;
    /*. += 0xC0000000;*/

    /*.text ALIGN(4k): AT(PREPAGE_END) */
    .text ALIGN(4k): 
    {
        *.o (.text);             
    }
    .data ALIGN(4K):
    {
        *.o (.data);
        *.o (.rodata*);
    }
    .bss ALIGN(4K):
    {
        *.o (.bss);
    }
    /DISCARD/ : 
    {
        *(.eh_frame);
        *(.comment);
    }
}
