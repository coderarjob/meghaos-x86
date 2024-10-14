
SECTIONS
{
    . = 0x00010000;

    .text : ALIGN(8)
    {
        /* '.entry.text' section must be at the top.
         * _start must have the address 0x00010000. */
        *(.entry.text);
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
