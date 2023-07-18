#include <unittest/fake.h>
#include <mock/kernel/x86/pmm.h>

DEFINE_FUNC_VOID_1(kpmm_arch_init, Bitmap *);
DEFINE_FUNC_0(U64, kpmm_arch_getInstalledMemoryByteCount);

void resetX86Pmm()
{
    RESET_FAKE(kpmm_arch_init);
    RESET_FAKE(kpmm_arch_getInstalledMemoryByteCount);
}
