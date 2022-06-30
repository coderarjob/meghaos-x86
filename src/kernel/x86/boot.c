/*
* --------------------------------------------------------------------------------------------------
* Megha Operating System V2 - x86 Kernel - Methods to access structures passed by the bootloader.
* --------------------------------------------------------------------------------------------------
*/
#include <kernel.h>
#include <boot_struct.h>

BootLoaderInfo* kboot_getCurrentBootLoaderInfo ()
{
    k_assert (BOOT_INFO_LOCATION, "BOOT INFO LOCATION is invalid");
    BootLoaderInfo* mi = (BootLoaderInfo*)BOOT_INFO_LOCATION;
    return mi;
}

U16 kboot_getBootLoaderInfoFilesCount (BootLoaderInfo const* bli)
{
    k_assert (bli, "BootLoaderInfo is NULL");
    return bli->filecount;
}

BootFileItem* kboot_getBootLoaderInfoBootFileItem (BootLoaderInfo const* bli, INT index)
{
    k_assert (bli, "BootLoaderInfo is NULL");
    k_assert (index >= 0, "Index Invalid");
    k_assert (index < bli->filecount, "Index invalid");

    return (BootFileItem *)&bli->files[index];
}

U16 kboot_getBootLoaderInfoBootMemoryMapItemCount (BootLoaderInfo const* bli)
{
    k_assert (bli, "BootLoaderInfo is NULL");
    return bli->count;
}

BootMemoryMapItem* kboot_getBootLoaderInfoBootMemoryMapItem (BootLoaderInfo const* bli, INT index)
{
    k_assert (bli, "BootLoaderInfo is NULL");
    k_assert (index >= 0, "Index Invalid");
    k_assert (index < bli->count, "Index invalid");

    return (BootMemoryMapItem *)&bli->items[index];
}

U32 kboot_getBootFileItemStartLocation (BootFileItem const* bfi)
{
    k_assert (bfi, "BootFileItem is NULL");
    return bfi->startLocation;
}

U16 kboot_getBootFileItemFileLength (BootFileItem const* bfi)
{
    k_assert (bfi, "BootFileItem is NULL");
    return bfi->length;
}

U64 kboot_getBootMemoryMapItemBaseAddress (BootMemoryMapItem const* bmmi)
{
    k_assert (bmmi, "BootMemoryMapItem is NULL");
    return bmmi->baseAddr;
}

U64 kboot_getBootMemoryMapItemLengthBytes (BootMemoryMapItem const* bmmi)
{
    k_assert (bmmi, "BootMemoryMapItem is NULL");
    return bmmi->length;
}

BootMemoryMapTypes kboot_getBootMemoryMapItemType (BootMemoryMapItem const* bmmi)
{
    k_assert (bmmi, "BootMemoryMapItem is NULL");
    return (BootMemoryMapTypes)bmmi->type;
}

ULLONG kboot_calculateAvailableMemory (BootLoaderInfo const* bli)
{
    k_assert (bli, "BootLoaderInfo is NULL");

    INT mapCount = bli->count;
    U64 length_bytes = 0;

    for (INT i = 0; i < mapCount; i++)
        length_bytes += bli->items[i].length;

    return length_bytes;
}
