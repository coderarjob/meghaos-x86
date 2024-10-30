#include <mock/kernel/x86/boot.h>
#include <unittest/fake.h>

DEFINE_FUNC (U16,kboot_getBootFileItemCount);
DEFINE_FUNC (BootFileItem,kboot_getBootFileItem, INT);
DEFINE_FUNC (BootMemoryMapItem,kboot_getBootMemoryMapItem, INT);
DEFINE_FUNC (U16,kboot_getBootMemoryMapItemCount);
DEFINE_FUNC (ULLONG,kboot_calculateInstalledMemory);
DEFINE_FUNC (BootGraphicsModeInfo,kboot_getGraphicsModeInfo);

void resetBootFake()
{
    RESET_FAKE (kboot_getBootFileItemCount);
    RESET_FAKE (kboot_getBootFileItem);
    RESET_FAKE (kboot_getBootMemoryMapItem);
    RESET_FAKE (kboot_getBootMemoryMapItemCount);
    RESET_FAKE (kboot_calculateInstalledMemory);
    RESET_FAKE (kboot_getGraphicsModeInfo);
}
