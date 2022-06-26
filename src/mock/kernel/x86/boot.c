#include <mock/kernel/x86/boot.h>
#include <unittest/fake.h>
#include <kernel.h>

DEFINE_FUNC_0 (BootLoaderInfo*, kboot_getCurrentBootLoaderInfo);
DEFINE_FUNC_1 (U16, kboot_getBootLoaderInfoFilesCount, IN, BootLoaderInfo *);
DEFINE_FUNC_1 (ULLONG, kboot_calculateAvailableMemory, IN, BootLoaderInfo *);

void resetBootFake()
{
    RESET_FAKE(kboot_getCurrentBootLoaderInfo);
    RESET_FAKE(kboot_getBootLoaderInfoFilesCount);
    RESET_FAKE(kboot_calculateAvailableMemory);
}
