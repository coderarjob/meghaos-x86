/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Bootloader
*
* Contains structures and methods that is used to get information from
* bootloader.
* ---------------------------------------------------------------------------
*
* Dated: 6th January 2020
*/

#ifndef BOOT_STRUCT_H_X86
#define BOOT_STRUCT_H_X86

    #include <types.h>
    #include <buildcheck.h>
    #include  <x86/boot.h>

    struct BootLoaderInfo
    {
        const struct GraphisModeInfo gxInfo;
        const U16 filecount;
        const struct BootFileItem files[11];
        const U16 count;
        const struct BootMemoryMapItem items[];
    }__attribute__ ((packed));

#endif //BOOT_STRUCT_H_X86
