#ifndef BOOT_FAKE_X86_H
#define BOOT_FAKE_X86_H

#include <unittest/fake.h>
#include <kernel.h>
#include <boot_struct.h>

DECLARE_FUNC_0 (BootLoaderInfo *, kboot_getCurrentBootLoaderInfo);
DECLARE_FUNC_1 (U16, kboot_getBootLoaderInfoFilesCount, IN, BootLoaderInfo const*);
DECLARE_FUNC_1 (ULLONG, kboot_calculateAvailableMemory, IN, BootLoaderInfo const*);

void resetBootFake();
#endif //BOOT_FAKE_X86_H
