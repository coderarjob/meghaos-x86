#include <unittest/fake.h>
#include <mock/kernel/pmm.h>

DEFINE_FUNC_VOID(kpmm_init);
DEFINE_FUNC(bool, kpmm_free, Physical, UINT);
DEFINE_FUNC(bool, kpmm_alloc, Physical*, UINT, KernelPhysicalMemoryRegions);
DEFINE_FUNC(bool, kpmm_allocAt, Physical, UINT, KernelPhysicalMemoryRegions);
DEFINE_FUNC(bool, kpmm_isInitialized);
DEFINE_FUNC(size_t,  kpmm_getFreeMemorySize);
DEFINE_FUNC(USYSINT, kpmm_getUsableMemorySize, KernelPhysicalMemoryRegions);

void resetPmm()
{
    RESET_FAKE(kpmm_init);
    RESET_FAKE(kpmm_free);
    RESET_FAKE(kpmm_alloc);
    RESET_FAKE(kpmm_allocAt);
    RESET_FAKE(kpmm_isInitialized);
    RESET_FAKE(kpmm_getFreeMemorySize);
    RESET_FAKE(kpmm_getUsableMemorySize);
}
