# Megha Operating System V2 - x86
--------------------------------------------------------------------------------

## Study on executable binary file format for the Kernel and other module files.
categories: note, independent
_22 April 2023_

The idea for choosing a file format goes to the colition-A bug. The bug resulted because the flat
kernel binary format did not have information about the size of the .BSS section. Read more about
the bug in the BUGS file.

The quick solution was to move the .BSS into the .data section. At the time it was fine and it
works. But in the future we have to use a more complex file format with support to .BSS.

Being in Linux, the first choice is to use the ELF format, its open and not much complex to
implement for my purpose. Another option is to have a flat binary with a very simple header.

Below is a comparison of each:

```
=================================================
.BSS section inside .DATA
-------------------------------------------------
Binary file type            Size (in bytes)
------------------------  ------------------------
ELF with debug symbols      79744
Flat binary                 12808
ELF stripped                Makes no sense to
                            have ELF and no .bss
=================================================

=================================================
.BSS section separated
-------------------------------------------------
Binary file type            Size (in bytes)
------------------------  ------------------------
ELF with debug symbols      79372
ELF stripped                12780
MOS Simple Executable*      12412 (Flat with header binary)
Flat binary                 Cannot be done. This
                            was the error case.
=================================================

(*) See below for header structure.
```

Firstly notice with separate .BSS section, both `ELF stripped` and `MOS Simple Executable` files are
smaller than the currently used `Flat binary` with .BSS in the .DATA section. This proves having a
separate .BSS section has advantages.

Now lets look at the two contenders.

The `ELF Stripped` is just 3.0% larger than the `MOS Simple Executable` binary.

So does it make any sense to have custom headers, when given the kernel size, the overhead of ELF
headers seem very small?

The stripped ELF only has `ELF headers`, `.prepage.text`, `.text`, `.data` and `.bss` sections. The
only section different in the two file formats are the headers.

Advantages on using ELF
* Its a standard file format and supports extra features, like permission bits for each section.
* Can contain debug info, which panic/debugger can use to display exact location.
* Bootloaders like GRUB also can be used.
* Be good to know a well known/used file format.
* Implementation in assembly can be fun.

Advantages of MOS Simple Executable:
* Simple yet exactly what is required. Have nothing extra.
* Debug information can be provided in a separate file. Need to explore this.
* Is not part of any standard, so can be molded for my needs.
* Can also be fun implementing a custom executable binary file format.

Conclusion:

1)
Nothing conclusive, I might do ELF first, to get an idea how complex it can be. ELF will take some
time as I have to read the ELF specification first. If the implementation is small enough there is
no need to do anything else.

One thing which I have to explore though, is how much it will effect the process model of Megha. I
do not want ELF to dictate my choices.

Another factor is what to do with debug information, where to keep them in memory after extracting
from the ELF binary and how to use it.

2)
Another option is the Kernel module to have the MOS Simple Executable format, while modules that
are loaded by kernel will all be ELF.

## MOS Simple Executable file header

```
    Offset Size Field
    in bin
    ------ ---- -----------
    0x0000  8   Magic
    0x0008  4   Header size
    0x000C  4   .text base
    0x0010  4   .text size
    0x0014  4   .data base
    0x0018  4   .data size
    0x001C  4   .bss base
    0x0020  4   .bss size
    0x0024  4   Start of .prepage.text

    Total: 36 bytes.
```

```
    KERNEL_PHYSICAL_START = 0x100000;
    KERNEL_VIRTUAL_START  = 0xC0000000;
    HEADER_LENGTH         = 36; /* Bytes */
    ...
    SECTIONS
    {
        . = KERNEL_PHYSICAL_START;                 /* Kernel is loaded at 100000 */
        .prepage.text :
        {
            *.o (.prepage.text);
        }
        . += KERNEL_VIRTUAL_START;

        <other sections not shown>

        . = KERNEL_PHYSICAL_START - HEADER_LENGTH;
        .fileheader :
        {
            LONG(0x4d4f535f);  /* "MOS_" */
            LONG(0x58534520);  /* "XSE " */

            LONG(HEADER_LENGTH);

            LONG(_text_start);
            LONG(_text_end - _text_start);

            LONG(_data_start);
            LONG(_data_end - _data_start);

            LONG(_bss_start);
            LONG(_bss_end - _bss_start);
        }
    }
```
