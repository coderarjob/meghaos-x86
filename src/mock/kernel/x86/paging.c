#define YUKTI_TEST_STRIP_PREFIX
#include <unittest/yukti.h>
#include <mock/kernel/x86/paging.h>

DEFINE_FUNC (ArchPageDirectoryEntry *, s_getPdeFromCurrentPd, UINT);
DEFINE_FUNC (ArchPageTableEntry *, s_getPteFromCurrentPd, UINT);
DEFINE_FUNC (void *, s_getLinearAddress, UINT, UINT, UINT);

void resetPagingFake()
{
    RESET_MOCK (s_getPteFromCurrentPd);
    RESET_MOCK (s_getPdeFromCurrentPd);
    RESET_MOCK (s_getLinearAddress);
}
