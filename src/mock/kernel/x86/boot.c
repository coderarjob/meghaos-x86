#include <mock/kernel/x86/boot.h>
#include <unittest/fake.h>
#include <kernel.h>

DEFINE_FUNC_0 (BootLoaderInfo*, kboot_getCurrentBootLoaderInfo);
DEFINE_FUNC_1 (U16, kboot_getBootLoaderInfoFilesCount, BootLoaderInfo const*);
DEFINE_FUNC_1 (ULLONG, kboot_calculateAvailableMemory, BootLoaderInfo const*);
DEFINE_FUNC_1 (U16, kboot_getBootLoaderInfoBootMemoryMapItemCount, BootLoaderInfo const*);
DEFINE_FUNC_1 (U16, kboot_getBootFileItemFileLength, BootFileItem const*);
DEFINE_FUNC_1 (U32, kboot_getBootFileItemStartLocation, BootFileItem const*);
DEFINE_FUNC_1 (BootMemoryMapTypes, kboot_getBootMemoryMapItemType, BootMemoryMapItem const*);
DEFINE_FUNC_1 (U64, kboot_getBootMemoryMapItemLengthBytes, BootMemoryMapItem const*);
DEFINE_FUNC_1 (U64, kboot_getBootMemoryMapItemBaseAddress, BootMemoryMapItem const*);

DEFINE_FUNC_2 (BootFileItem*
               , kboot_getBootLoaderInfoBootFileItem
               , BootLoaderInfo const*
               , INT);
DEFINE_FUNC_2 (BootMemoryMapItem*
               , kboot_getBootLoaderInfoBootMemoryMapItem
               , BootLoaderInfo const*
               , INT);
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
