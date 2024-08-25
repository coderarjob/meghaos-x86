#pragma once

#include <unittest/fake.h>
#include <types.h>
#include <vmm.h>

DECLARE_FUNC(PTR, kvmm_memmap, VMemoryManager*, PTR, Physical const* const, SIZE,
                 VMemoryMemMapFlags, Physical* const );

void resetVMMFake();
