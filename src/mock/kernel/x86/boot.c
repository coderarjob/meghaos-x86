#include <mock/kernel/x86/boot.h>
#include <unittest/fake.h>
#include <kernel.h>

DEFINE_FUNC_0 (BootLoaderInfo*, kboot_getCurrentBootLoaderInfo);
DEFINE_FUNC_1 (U16, kboot_getBootLoaderInfoFilesCount, IN, BootLoaderInfo const*);
DEFINE_FUNC_1 (ULLONG, kboot_calculateAvailableMemory, IN, BootLoaderInfo const*);
DEFINE_FUNC_1 (U16, kboot_getBootLoaderInfoBootMemoryMapItemCount, IN, BootLoaderInfo const*);
DEFINE_FUNC_1 (U16, kboot_getBootFileItemFileLength, IN, BootFileItem const*);
DEFINE_FUNC_1 (U32, kboot_getBootFileItemStartLocation, IN, BootFileItem const*);
DEFINE_FUNC_1 (BootMemoryMapTypes, kboot_getBootMemoryMapItemType, IN, BootMemoryMapItem const*);
DEFINE_FUNC_1 (U64, kboot_getBootMemoryMapItemLengthBytes, IN, BootMemoryMapItem const*);
DEFINE_FUNC_1 (U64, kboot_getBootMemoryMapItemBaseAddress, IN, BootMemoryMapItem const*);

DEFINE_FUNC_2 (BootFileItem*
               , kboot_getBootLoaderInfoBootFileItem
               , IN, BootLoaderInfo const*
               , IN, INT);
DEFINE_FUNC_2 (BootMemoryMapItem*
               , kboot_getBootLoaderInfoBootMemoryMapItem
               , IN, BootLoaderInfo const*
               , IN, INT);
void resetBootFake()
{
    RESET_FAKE(kboot_getCurrentBootLoaderInfo);
    RESET_FAKE(kboot_getBootLoaderInfoFilesCount);
    RESET_FAKE(kboot_calculateAvailableMemory);
    RESET_FAKE(kboot_getBootLoaderInfoBootFileItem);
    RESET_FAKE(kboot_getBootLoaderInfoBootMemoryMapItem);
    RESET_FAKE(kboot_getBootLoaderInfoBootMemoryMapItemCount);
    RESET_FAKE(kboot_getBootFileItemFileLength);
    RESET_FAKE(kboot_getBootFileItemStartLocation);
    RESET_FAKE(kboot_getBootMemoryMapItemType);
    RESET_FAKE(kboot_getBootMemoryMapItemLengthBytes);
    RESET_FAKE(kboot_getBootMemoryMapItemBaseAddress);
}
