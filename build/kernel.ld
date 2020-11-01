ENTRY (__main)
SECTIONS
{
    . = 0x28000;                 /* Kernel is loaded at 0x1800:0x0000 */
    .text :AT(0x0)
    {
        *.o (.text);
    }
    .data :
    {
        *.o (.data);
        *.o (.bss);
        *.o (.rodata);
    }
    /DISCARD/ : 
    {
        *(.eh_frame)
    }
}
