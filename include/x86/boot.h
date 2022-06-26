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

    U16 kboot_getBootLoaderInfoFilesCount (BootLoaderInfo const* bli);
    BootFileItem* kboot_getBootLoaderInfoBootFileItem (BootLoaderInfo const* bli, INT index);
    BootMemoryMapItem* kboot_getBootLoaderInfoBootMemoryMapItem (BootLoaderInfo const* bli, INT index);
    U16 kboot_getBootLoaderInfoBootMemoryMapItemCount (BootLoaderInfo const* bli);

    U16 kboot_getBootFileItemFileLength (BootFileItem const* bfi);
    U32 kboot_getBootFileItemStartLocation (BootFileItem const* bfi);

    BootMemoryMapTypes kboot_getBootMemoryMapItemType (BootMemoryMapItem const* bmmi);
    U64 kboot_getBootMemoryMapItemLengthBytes (BootMemoryMapItem const* bmmi);
    U64 kboot_getBootMemoryMapItemBaseAddress (BootMemoryMapItem const* bmmi);

    ULLONG kboot_calculateAvailableMemory (BootLoaderInfo const* bli);
#endif //BOOT_H
