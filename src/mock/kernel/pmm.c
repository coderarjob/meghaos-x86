#include <unittest/fake.h>
#include <mock/kernel/pmm.h>

DEFINE_FUNC_VOID_0(kpmm_init);
DEFINE_FUNC_2(bool, kpmm_free, Physical, UINT);
DEFINE_FUNC_2(Physical, kpmm_alloc, UINT, bool);
DEFINE_FUNC_3(bool,  kpmm_allocAt, Physical, UINT, bool);
DEFINE_FUNC_0(bool,  kpmm_isInitialized);
DEFINE_FUNC_0(size_t,  kpmm_getFreeMemorySize);
DEFINE_FUNC_1(UINT,  kpmm_getAddressablePageCount, bool);

void resetPmm()
{
    RESET_FAKE(kpmm_init);
    RESET_FAKE(kpmm_free);
    RESET_FAKE(kpmm_alloc);
    RESET_FAKE(kpmm_allocAt);
    RESET_FAKE(kpmm_isInitialized);
    RESET_FAKE(kpmm_getFreeMemorySize);
    RESET_FAKE(kpmm_getAddressablePageCount);
}
