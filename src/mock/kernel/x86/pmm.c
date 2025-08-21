#define YUKTI_TEST_STRIP_PREFIX
#include <unittest/yukti.h>
#include <mock/kernel/x86/pmm.h>

DEFINE_FUNC_VOID(kpmm_arch_init, Bitmap *);
DEFINE_FUNC(U64, kpmm_arch_getInstalledMemoryByteCount);

void resetX86Pmm()
{
    RESET_MOCK(kpmm_arch_init);
    RESET_MOCK(kpmm_arch_getInstalledMemoryByteCount);
}
