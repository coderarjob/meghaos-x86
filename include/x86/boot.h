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

    struct mem_des
    {
        u64 baseAddr;
        u64 length;
        u32 type;
    }__attribute__((packed));
    
    struct file_des
    {
        u32 startLocation;
        u16 length;
    }__attribute__((packed));

    struct boot_info
    {
        u16 filecount;
        struct file_des files[11];
        u16 count;
        struct mem_des items[];
    }__attribute__((packed));

#endif //__BOOT_H__
