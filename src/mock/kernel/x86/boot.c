#include <mock/kernel/x86/boot.h>
#include <unittest/fake.h>

DEFINE_FUNC_0 (BootLoaderInfo*, kboot_getCurrentBootLoaderInfo);
DEFINE_FUNC_1 (ULLONG, kboot_calculateAvailableMemory, BootLoaderInfo const*);

DEFINE_FUNC_1 (U16, kBootFileItem_getLength, BootFileItem const*);
DEFINE_FUNC_1 (U32, kBootFileItem_getStartLocation, BootFileItem const*);

DEFINE_FUNC_1 (BootMemoryMapTypes, kBootMemoryMapItem_getType, BootMemoryMapItem const*);
DEFINE_FUNC_1 (U64, kBootMemoryMapItem_getLength, BootMemoryMapItem const*);
DEFINE_FUNC_1 (U64, kBootMemoryMapItem_getBaseAddress, BootMemoryMapItem const*);

DEFINE_FUNC_1 (U16, kBootLoaderInfo_getMemoryMapItemCount, BootLoaderInfo const*);
DEFINE_FUNC_1 (U16, kBootLoaderInfo_getFilesCount, BootLoaderInfo const*);
DEFINE_FUNC_2 (BootFileItem*, kBootLoaderInfo_getFileItem, BootLoaderInfo const*, INT);
DEFINE_FUNC_2 (BootMemoryMapItem*, kBootLoaderInfo_getMemoryMapItem, BootLoaderInfo const*, INT);

void resetBootFake()
{
    RESET_FAKE(kboot_getCurrentBootLoaderInfo);
    RESET_FAKE(kBootLoaderInfo_getFilesCount);
    RESET_FAKE(kboot_calculateAvailableMemory);
    RESET_FAKE(kBootLoaderInfo_getFileItem);
    RESET_FAKE(kBootLoaderInfo_getMemoryMapItem);
    RESET_FAKE(kBootLoaderInfo_getMemoryMapItemCount);
    RESET_FAKE(kBootFileItem_getLength);
    RESET_FAKE(kBootFileItem_getStartLocation);
    RESET_FAKE(kBootMemoryMapItem_getType);
    RESET_FAKE(kBootMemoryMapItem_getLength);
    RESET_FAKE(kBootMemoryMapItem_getBaseAddress);
}
