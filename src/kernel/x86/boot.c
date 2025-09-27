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
#include <kstdlib.h>

static BootLoaderInfo* kboot_getCurrentBootLoaderInfo(void)
{
    k_assert (MEM_START_BOOT_INFO, "BOOT INFO LOCATION is invalid");
    BootLoaderInfo* mi = (BootLoaderInfo*)MEM_START_BOOT_INFO;
    return mi;
}

static void convert_to_f12_filename (const char* fn, char* out)
{
    int di = 0;
    for (char c = *fn; c != '\0' && di < 11; di++, c = *(++fn)) {
        if (c == '.') {
            for (; di < 8; di++) {
                *out++ = ' ';
            }
            di--;
        } else {
            *out++ = c;
        }
    }

    while (di++ < 11) {
        *out++ = ' ';
    }
}

U16 kboot_getBootFileItemCount(void)
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

BootFileItem kboot_findBootFileItem (const CHAR* const filename)
{
    CHAR fat12FileName[CONFIG_BOOT_FILENAME_LEN_CHARS] = { 0 };
    convert_to_f12_filename (filename, fat12FileName);
    k_assert (k_strlen (fat12FileName) == 11, "Invalid file name");

    const BootFileItem* bfi = NULL;
    BootLoaderInfo* bli     = kboot_getCurrentBootLoaderInfo();
    for (int i = 0; i < bli->filecount; i++) {
        bfi = &bli->files[i];
        if (k_memcmp (bfi->name, fat12FileName, CONFIG_BOOT_FILENAME_LEN_CHARS)) {
            return *bfi;
        }
    }
    k_assert (false, "Invalid file name");
    NORETURN();
}

U16 kboot_getBootMemoryMapItemCount(void)
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

ULLONG kboot_calculateInstalledMemory(void)
{
    BootLoaderInfo* bli = kboot_getCurrentBootLoaderInfo();

    INT mapCount     = bli->count;
    U64 length_bytes = 0;

    for (INT i = 0; i < mapCount; i++)
        length_bytes += bli->items[i].length;

    return length_bytes;
}

BootGraphicsModeInfo kboot_getGraphicsModeInfo(void)
{
    return kboot_getCurrentBootLoaderInfo()->gxInfo;
}

const U8* kboot_getFontData(void)
{
    return kboot_getCurrentBootLoaderInfo()->font_data;
}
