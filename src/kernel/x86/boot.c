/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - Methods to access structures passed by the bootloader.
 * --------------------------------------------------------------------------------------------------
 */
#include <kassert.h>
#include <x86/boot.h>
#include <x86/memloc.h>
#include <x86/boot_struct.h>
#include <kdebug.h>

static BootLoaderInfo* kboot_getCurrentBootLoaderInfo()
{
    k_assert (MEM_START_BOOT_INFO, "BOOT INFO LOCATION is invalid");
    BootLoaderInfo* mi = (BootLoaderInfo*)MEM_START_BOOT_INFO;
    return mi;
}

U16 kboot_getBootFileItemCount()
{
    return kboot_getCurrentBootLoaderInfo()->filecount;
}

BootFileItem kboot_getBootFileItem (INT index)
{
    BootLoaderInfo* bli = kboot_getCurrentBootLoaderInfo();
    k_assert (index >= 0, "Index Invalid");
    k_assert (index < bli->filecount, "Index invalid");

    return bli->files[index];
}

U16 kboot_getBootMemoryMapItemCount()
{
    return kboot_getCurrentBootLoaderInfo()->count;
}

BootMemoryMapItem kboot_getBootMemoryMapItem (INT index)
{
    BootLoaderInfo* bli = kboot_getCurrentBootLoaderInfo();

    k_assert (index >= 0, "Index Invalid");
    k_assert (index < bli->count, "Index invalid");

    return bli->items[index];
}

ULLONG kboot_calculateInstalledMemory()
{
    BootLoaderInfo* bli = kboot_getCurrentBootLoaderInfo();

    INT mapCount     = bli->count;
    U64 length_bytes = 0;

    for (INT i = 0; i < mapCount; i++)
        length_bytes += bli->items[i].length;

    return length_bytes;
}

GraphisModeInfo kboot_getGraphicsModeInfo()
{
    return kboot_getCurrentBootLoaderInfo()->gxInfo;
}
