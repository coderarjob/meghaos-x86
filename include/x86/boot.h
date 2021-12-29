/*
* ---------------------------------------------------------------------------
* Megha Operating System V2 - x86 Bootloader 
*
* Contains structures and methods that is used to get information from
* bootloder.
* ---------------------------------------------------------------------------
*
* Dated: 6th January 2020
*/

#ifndef __BOOT_H__
#define __BOOT_H__

    #include <types.h>

    typedef struct BootMemoryMapItem
    {
        U64 baseAddr;
        U64 length;
        U32 type;
    }__attribute__ ((packed)) BootMemoryMapItem;
    
    typedef struct BootFileItem
    {
        U32 startLocation;
        U16 length;
    }__attribute__ ((packed)) BootFileItem;

    typedef struct BootLoaderInfo
    {
        U16 filecount;
        BootFileItem files[11];
        U16 count;
        BootMemoryMapItem items[];
    }__attribute__ ((packed)) BootLoaderInfo;

#endif //__BOOT_H__
