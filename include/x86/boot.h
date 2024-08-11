/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - x86 Bootloader 
*
* Contains abstract structure declarations and methods that is used to get information from the
* bootloader.
* --------------------------------------------------------------------------------------------------
*/

#ifndef BOOT_H_X86
#define BOOT_H_X86

    #include <types.h>
    #include <buildcheck.h>

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

    U16 kBootLoaderInfo_getFilesCount (BootLoaderInfo const* bli);
    BootFileItem* kBootLoaderInfo_getFileItem (BootLoaderInfo const* bli, INT index);
    BootMemoryMapItem* kBootLoaderInfo_getMemoryMapItem (BootLoaderInfo const* bli, INT index);
    U16 kBootLoaderInfo_getMemoryMapItemCount (BootLoaderInfo const* bli);

    U16 kBootFileItem_getLength (BootFileItem const* bfi);
    U32 kBootFileItem_getStartLocation (BootFileItem const* bfi);

    BootMemoryMapTypes kBootMemoryMapItem_getType (BootMemoryMapItem const* bmmi);
    U64 kBootMemoryMapItem_getLength (BootMemoryMapItem const* bmmi);
    U64 kBootMemoryMapItem_getBaseAddress (BootMemoryMapItem const* bmmi);

    ULLONG kboot_calculateAvailableMemory (BootLoaderInfo const* bli);

    Physical kboot_checkGraphicsModeInfo(BootLoaderInfo const *bli);
#endif //BOOT_H_X86
