#define YUKTI_TEST_STRIP_PREFIX
#include <unittest/yukti.h>
#include <mock/kernel/pmm.h>

/* DEFINE_FUNC_*DONT_MOCK is used for functions which have non integer arguments. yukti.h does not
 * support mocking for such functions. */

DEFINE_FUNC_VOID(kpmm_init);
DEFINE_FUNC_FALLBACK(bool, kpmm_free, Physical, UINT);
DEFINE_FUNC(bool, kpmm_alloc, Physical*, UINT, KernelPhysicalMemoryRegions);
DEFINE_FUNC_FALLBACK(bool, kpmm_allocAt, Physical, UINT, KernelPhysicalMemoryRegions);
DEFINE_FUNC(size_t,  kpmm_getFreeMemorySize);
DEFINE_FUNC(USYSINT, kpmm_getUsableMemorySize, KernelPhysicalMemoryRegions);

void resetPmm(void)
{
    RESET_MOCK(kpmm_init);
    RESET_MOCK(kpmm_free);
    RESET_MOCK(kpmm_alloc);
    RESET_MOCK(kpmm_allocAt);
    RESET_MOCK(kpmm_getFreeMemorySize);
    RESET_MOCK(kpmm_getUsableMemorySize);
}
