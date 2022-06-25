/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - x86 Bootloader 
*
* Contains abstract structure declarations and methods that is used to get information from the
* bootloader.
* --------------------------------------------------------------------------------------------------
*/

#ifndef BOOT_H
#define BOOT_H

    #include <types.h>
    typedef enum BootMemoryMapTypes
    {
        MMTYPE_FREE = 1,
        MMTYPE_RESERVED = 2,
        MMTYPE_ACPI_RECLAIM = 3,
    } BootMemoryMapTypes;

    typedef struct BootMemoryMapItem BootMemoryMapItem;
    typedef struct BootFileItem BootFileItem;
    typedef struct BootLoaderInfo BootLoaderInfo;

    BootLoaderInfo* kboot_getCurrentBootLoaderInfo ();

    U16 kboot_getBootLoaderInfoFilesCount (BootLoaderInfo *bli);
    BootFileItem* kboot_getBootLoaderInfoBootFileItem (BootLoaderInfo *bli, INT index);
    BootMemoryMapItem* kboot_getBootLoaderInfoBootMemoryMapItem (BootLoaderInfo *bli, INT index);
    U16 kboot_getBootLoaderInfoBootMemoryMapItemCount (BootLoaderInfo *bli);

    U16 kboot_getBootFileItemFileLength (BootFileItem *bfi);
    U32 kboot_getBootFileItemStartLocation (BootFileItem *bfi);

    BootMemoryMapTypes kboot_getBootMemoryMapItemType (BootMemoryMapItem *bmmi);
    U64 kboot_getBootMemoryMapItemLengthBytes (BootMemoryMapItem *bmmi);
    U64 kboot_getBootMemoryMapItemBaseAddress (BootMemoryMapItem *bmmi);

    ULLONG kboot_calculateAvailableMemory (BootLoaderInfo *bli);
#endif //BOOT_H
