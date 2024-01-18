#include <unittest/fake.h>
#include <mock/kernel/x86/paging.h>

DEFINE_FUNC (ArchPageDirectoryEntry *, s_getPDE, UINT);
DEFINE_FUNC (ArchPageTableEntry *, s_getPTE, UINT);

void resetPagingFake()
{
    RESET_FAKE (s_getPTE);
    RESET_FAKE (s_getPDE);
}
