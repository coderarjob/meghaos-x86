#ifndef PAGING_FAKE_X86_H
#define PAGING_FAKE_X86_H

#define YUKTI_TEST_STRIP_PREFIX
#include <unittest/yukti.h>
#include <paging.h>

DECLARE_FUNC(ArchPageDirectoryEntry*, s_getPdeFromCurrentPd, UINT);
DECLARE_FUNC(ArchPageTableEntry*, s_getPteFromCurrentPd, UINT);
DECLARE_FUNC (void *, s_getLinearAddress, UINT, UINT, UINT);

void resetPagingFake();

#endif // PAGING_FAKE_X86_H
