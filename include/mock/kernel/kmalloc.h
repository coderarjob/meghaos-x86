#ifndef KMALLOC_FAKE_H
#define KMALLOC_FAKE_H

#include <unittest/fake.h>

DECLARE_FUNC(void*, s_PreAllocateMemory);

void resetKmallocFake();

#endif // KMALLOC_FAKE_H
