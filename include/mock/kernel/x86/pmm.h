#ifndef PMM_FAKE_X86_H
#define PMM_FAKE_X86_H

#include <unittest/fake.h>
#include <types.h>

DECLARE_FUNC_VOID(kpmm_arch_init, U8 *);
DECLARE_FUNC(U64, kpmm_arch_getInstalledMemoryByteCount);

void resetX86Pmm();

#endif // PMM_FAKE_X86_H
