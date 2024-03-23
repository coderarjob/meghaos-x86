#include <unittest/fake.h>
#include <mock/kernel/x86/paging.h>

DEFINE_FUNC (ArchPageDirectoryEntry *, s_getPdeFromCurrentPd, UINT);
DEFINE_FUNC (ArchPageTableEntry *, s_getPteFromCurrentPd, UINT);
DEFINE_FUNC (void *, s_getLinearAddress, UINT, UINT, UINT);

void resetPagingFake()
{
    RESET_FAKE (s_getPteFromCurrentPd);
    RESET_FAKE (s_getPdeFromCurrentPd);
    RESET_FAKE (s_getLinearAddress);
}
