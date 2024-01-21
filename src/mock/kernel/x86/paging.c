#include <unittest/fake.h>
#include <mock/kernel/x86/paging.h>

DEFINE_FUNC (ArchPageDirectoryEntry *, s_getPdeFromCurrentPd, UINT);
DEFINE_FUNC (ArchPageTableEntry *, s_getPteFromCurrentPd, UINT);

void resetPagingFake()
{
    RESET_FAKE (s_getPteFromCurrentPd);
    RESET_FAKE (s_getPdeFromCurrentPd);
}
