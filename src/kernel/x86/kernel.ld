
SECTIONS
{
    . = 0x100000;                 /* Kernel is loaded at 100000 */
    .prepage.text : 
    {
        *(.prepage.text);
    }
    . += 0xC0000000; 

    .text : AT( ADDR(.text) - 0xC0000000)
    {
        *(.text);             
    }
    .data : ALIGN(8)
    {
        *(.data);

        . = ALIGN(8);
        *(.rodata*);

        . = ALIGN(8);
        *(.bss);
        *(COMMON);
    }

    /DISCARD/ : 
    {
        *(.eh_frame);
        *(.comment);
    }
}
