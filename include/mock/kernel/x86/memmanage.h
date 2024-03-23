#ifndef KMALLOC_FAKE_H
#define KMALLOC_FAKE_H

#include <unittest/fake.h>

DECLARE_FUNC(void*, kmalloc_arch_preAllocateMemory);
DECLARE_FUNC(void*, salloc_arch_preAllocateMemory);

void resetX86MemManageFake();

#endif // KMALLOC_FAKE_H
