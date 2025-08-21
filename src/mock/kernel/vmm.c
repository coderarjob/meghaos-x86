#define YUKTI_TEST_STRIP_PREFIX
#include <unittest/yukti.h>
#include <mock/kernel/vmm.h>

DEFINE_FUNC (PTR, kvmm_memmap, VMemoryManager* , PTR , Physical const* const, SIZE,
                 VMemoryMemMapFlags , Physical* const );

void resetVMMFake()
{
    RESET_MOCK(kvmm_memmap);
}
