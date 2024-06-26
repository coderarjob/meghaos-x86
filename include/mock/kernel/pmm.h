#ifndef PMM_FAKE_H
#define PMM_FAKE_H

#include <unittest/fake.h>
#include <types.h>
#include <pmm.h>

DECLARE_FUNC_VOID(kpmm_init);
DECLARE_FUNC(bool, kpmm_free, Physical, UINT);
DECLARE_FUNC(bool, kpmm_alloc, Physical*, UINT, KernelPhysicalMemoryRegions);
DECLARE_FUNC(bool, kpmm_allocAt, Physical, UINT, KernelPhysicalMemoryRegions);
DECLARE_FUNC(size_t,  kpmm_getFreeMemorySize);
DECLARE_FUNC(USYSINT, kpmm_getUsableMemorySize, KernelPhysicalMemoryRegions);

void resetPmm();

#endif // PMM_FAKE_H
