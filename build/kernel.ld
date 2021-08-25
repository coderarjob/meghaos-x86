ENTRY (__kernel_entry)
SECTIONS
{
    . = 0x100000;                 /* Kernel is loaded at 100000 */
    .prepage.text : 
    {
        *.o (.prepage.text);
    }
    .prepage.bss :
    {
        *.o (.prepage.bss);
    }
    PREPAGE_END = .;
    . += 0xC0000000; 

    .text : AT(PREPAGE_END)
    {
        *.o (.text);             
    }
    .data :
    {
        *.o (.data);
        *.o (.rodata*);
    }
    .bss :
    {
        *.o (.bss);
    }
    /DISCARD/ : 
    {
        *(.eh_frame);
        *(.comment);
    }
}
