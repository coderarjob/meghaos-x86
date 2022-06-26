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

#ifndef BOOT_STRUCT_H
#define BOOT_STRUCT_H

    #include <kernel.h>

    struct BootMemoryMapItem
    {
        U64 baseAddr;
        U64 length;
        U32 type;
    }__attribute__ ((packed));

    struct BootFileItem
    {
        U32 startLocation;
        U16 length;
    }__attribute__ ((packed));

    struct BootLoaderInfo
    {
        U16 filecount;
        BootFileItem files[11];
        U16 count;
        BootMemoryMapItem items[];
    }__attribute__ ((packed));

#endif //BOOT_STRUCT_H
