#ifndef BOOT_FAKE_X86_H
#define BOOT_FAKE_X86_H

#include <unittest/fake.h>
#include <kernel.h>
#include <boot_struct.h>

DECLARE_FUNC (BootLoaderInfo *, kboot_getCurrentBootLoaderInfo);
DECLARE_FUNC (U16, kboot_getBootLoaderInfoFilesCount, BootLoaderInfo const*);
DECLARE_FUNC (ULLONG, kboot_calculateAvailableMemory, BootLoaderInfo const*);
DECLARE_FUNC (BootFileItem*
             , kboot_getBootLoaderInfoBootFileItem
             , BootLoaderInfo const*
             , INT);
DECLARE_FUNC (BootMemoryMapItem*
             , kboot_getBootLoaderInfoBootMemoryMapItem
             , BootLoaderInfo const*
             , INT);
DECLARE_FUNC (U16, kboot_getBootLoaderInfoBootMemoryMapItemCount, BootLoaderInfo const*);
DECLARE_FUNC (U16, kboot_getBootFileItemFileLength, BootFileItem const*);
DECLARE_FUNC (U32, kboot_getBootFileItemStartLocation, BootFileItem const*);
DECLARE_FUNC (BootMemoryMapTypes, kboot_getBootMemoryMapItemType, BootMemoryMapItem const*);
DECLARE_FUNC (U64, kboot_getBootMemoryMapItemLengthBytes, BootMemoryMapItem const*);
DECLARE_FUNC (U64, kboot_getBootMemoryMapItemBaseAddress, BootMemoryMapItem const*);

void resetBootFake();
#endif //BOOT_FAKE_X86_H
