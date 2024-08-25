#include <unittest/unittest.h>
#include <mock/kernel/vmm.h>

DEFINE_FUNC (PTR, kvmm_memmap, VMemoryManager* , PTR , Physical const* const, SIZE,
                 VMemoryMemMapFlags , Physical* const );

void resetVMMFake()
{
    RESET_FAKE(kvmm_memmap);
}
