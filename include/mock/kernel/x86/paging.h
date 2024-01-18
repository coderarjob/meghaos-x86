#ifndef PAGING_FAKE_X86_H
#define PAGING_FAKE_X86_H

#include <unittest/fake.h>
#include <paging.h>

DECLARE_FUNC(ArchPageDirectoryEntry*, s_getPDE, UINT);
DECLARE_FUNC(ArchPageTableEntry*, s_getPTE, UINT);

void resetPagingFake();

#endif // PAGING_FAKE_X86_H
