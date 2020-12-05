ENTRY (__main)
SECTIONS
{
    . = 0x28000;                 /* Kernel is loaded at 0x2800:0x0000 */
    .text :AT(0x0)
    {
        /* Order of linking .o files is importaint. 
         * We must link kernel first. The first function in kernel.o must be
         * __main() */
        build/obj/kernel/x86_kernel.o (.text);
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
