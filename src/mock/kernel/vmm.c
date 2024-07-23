#include <unittest/unittest.h>
#include <mock/kernel/vmm.h>

DEFINE_FUNC(PTR, kvmm_alloc, VMemoryManager*, SIZE, PagingMapFlags);

void resetVMMFake()
{
    RESET_FAKE(kvmm_alloc);
}
