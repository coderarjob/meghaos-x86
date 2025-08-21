#define YUKTI_TEST_STRIP_PREFIX
#include <unittest/yukti.h>
#include <mock/kernel/x86/boot.h>

DEFINE_FUNC (U16,kboot_getBootFileItemCount);
DEFINE_FUNC (BootFileItem,kboot_getBootFileItem, INT);
DEFINE_FUNC (BootMemoryMapItem,kboot_getBootMemoryMapItem, INT);
DEFINE_FUNC (U16,kboot_getBootMemoryMapItemCount);
DEFINE_FUNC (ULLONG,kboot_calculateInstalledMemory);
DEFINE_FUNC (BootGraphicsModeInfo,kboot_getGraphicsModeInfo);

void resetBootFake()
{
    RESET_MOCK (kboot_getBootFileItemCount);
    RESET_MOCK (kboot_getBootFileItem);
    RESET_MOCK (kboot_getBootMemoryMapItem);
    RESET_MOCK (kboot_getBootMemoryMapItemCount);
    RESET_MOCK (kboot_calculateInstalledMemory);
    RESET_MOCK (kboot_getGraphicsModeInfo);
}
