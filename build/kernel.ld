
SECTIONS
{
    . = 0x100000;                 /* Kernel is loaded at 100000 */
    .prepage.text : 
    {
        *.o (.prepage.text);
    }
    . += 0xC0000000; 

    .text : AT( ADDR(.text) - 0xC0000000)
    {
        *.o (.text);             
    }
    .data : ALIGN(8)
    {
        *.o (.data);

        . = ALIGN(8);
        *.o (.rodata*);

        . = ALIGN(8);
        *.o (.bss);
        *.o (COMMON);
    }

    /DISCARD/ : 
    {
        *(.eh_frame);
        *(.comment);
    }
}
