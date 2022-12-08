#ifndef BOOT_FAKE_X86_H
#define BOOT_FAKE_X86_H

#include <unittest/fake.h>
#include <kernel.h>

DECLARE_FUNC (BootLoaderInfo *, kboot_getCurrentBootLoaderInfo);
DECLARE_FUNC (ULLONG, kboot_calculateAvailableMemory, BootLoaderInfo const*);

DECLARE_FUNC (U16, kBootFileItem_getLength, BootFileItem const*);
DECLARE_FUNC (U32, kBootFileItem_getStartLocation, BootFileItem const*);

DECLARE_FUNC (BootMemoryMapTypes, kBootMemoryMapItem_getType, BootMemoryMapItem const*);
DECLARE_FUNC (U64, kBootMemoryMapItem_getLength, BootMemoryMapItem const*);
DECLARE_FUNC (U64, kBootMemoryMapItem_getBaseAddress, BootMemoryMapItem const*);

DECLARE_FUNC (U16, kBootLoaderInfo_getFilesCount, BootLoaderInfo const*);
DECLARE_FUNC (BootFileItem*, kBootLoaderInfo_getFileItem, BootLoaderInfo const*, INT);
DECLARE_FUNC (BootMemoryMapItem*, kBootLoaderInfo_getMemoryMapItem, BootLoaderInfo const*, INT);
DECLARE_FUNC (U16, kBootLoaderInfo_getMemoryMapItemCount, BootLoaderInfo const*);

void resetBootFake();
#endif //BOOT_FAKE_X86_H
