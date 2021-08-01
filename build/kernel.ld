ENTRY (__kernel_entry)
SECTIONS
{
    . = 0x28000;                 /* Kernel is loaded at 0x2800:0x0000 */
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
