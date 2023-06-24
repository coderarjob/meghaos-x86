#ifndef PMM_FAKE_H
#define PMM_FAKE_H

#include <unittest/fake.h>
#include <types.h>

DECLARE_FUNC_VOID(kpmm_init);
DECLARE_FUNC(bool, kpmm_free, Physical, UINT);
DECLARE_FUNC(Physical, kpmm_alloc, UINT, bool);
DECLARE_FUNC(bool,  kpmm_allocAt, Physical, UINT, bool);
DECLARE_FUNC(bool,  kpmm_isInitialized);
DECLARE_FUNC(size_t,  kpmm_getFreeMemorySize);
DECLARE_FUNC(UINT,  kpmm_getAddressablePageCount, bool);

void resetPmm();

#endif // PMM_FAKE_H
