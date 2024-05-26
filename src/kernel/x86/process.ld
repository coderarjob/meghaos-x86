
SECTIONS
{
    . = 0x00010000;

    .text : ALIGN(8)
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
