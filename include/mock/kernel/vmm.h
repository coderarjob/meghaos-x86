#pragma once

#include <unittest/fake.h>
#include <types.h>
#include <vmm.h>

DECLARE_FUNC(PTR, kvmm_alloc, VMemoryManager*, SIZE, PagingMapFlags);

void resetVMMFake();
