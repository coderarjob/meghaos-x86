#ifndef BOOT_FAKE_X86_H
#define BOOT_FAKE_X86_H

#include <unittest/fake.h>
#include <kernel.h>
#include <boot_struct.h>

DECLARE_FUNC_0 (BootLoaderInfo *, kboot_getCurrentBootLoaderInfo);
DECLARE_FUNC_1 (U16, kboot_getBootLoaderInfoFilesCount, IN, BootLoaderInfo const*);
DECLARE_FUNC_1 (ULLONG, kboot_calculateAvailableMemory, IN, BootLoaderInfo const*);
DECLARE_FUNC_2 (BootFileItem*
               , kboot_getBootLoaderInfoBootFileItem
               , IN, BootLoaderInfo const*
               , IN, INT);
DECLARE_FUNC_2 (BootMemoryMapItem*
               , kboot_getBootLoaderInfoBootMemoryMapItem
               , IN, BootLoaderInfo const*
               , IN, INT);
DECLARE_FUNC_1 (U16, kboot_getBootLoaderInfoBootMemoryMapItemCount, IN, BootLoaderInfo const*);
DECLARE_FUNC_1 (U16, kboot_getBootFileItemFileLength, IN, BootFileItem const*);
DECLARE_FUNC_1 (U32, kboot_getBootFileItemStartLocation, IN, BootFileItem const*);
DECLARE_FUNC_1 (BootMemoryMapTypes, kboot_getBootMemoryMapItemType, IN, BootMemoryMapItem const*);
DECLARE_FUNC_1 (U64, kboot_getBootMemoryMapItemLengthBytes, IN, BootMemoryMapItem const*);
DECLARE_FUNC_1 (U64, kboot_getBootMemoryMapItemBaseAddress, IN, BootMemoryMapItem const*);

void resetBootFake();
#endif //BOOT_FAKE_X86_H
