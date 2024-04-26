
SECTIONS
{
    . = 0x00010000; 

    .text : ALIGN(8)
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
