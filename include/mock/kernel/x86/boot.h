#ifndef BOOT_FAKE_X86_H
#define BOOT_FAKE_X86_H

#include <unittest/fake.h>
#include <x86/boot.h>

DECLARE_FUNC (U16, kboot_getBootFileItemCount);
DECLARE_FUNC (BootFileItem,kboot_getBootFileItem, INT);
DECLARE_FUNC (BootMemoryMapItem,kboot_getBootMemoryMapItem, INT);
DECLARE_FUNC (U16,kboot_getBootMemoryMapItemCount);
DECLARE_FUNC (ULLONG,kboot_calculateInstalledMemory);
DECLARE_FUNC (BootGraphicsModeInfo,kboot_getGraphicsModeInfo);

void resetBootFake();
#endif //BOOT_FAKE_X86_H
