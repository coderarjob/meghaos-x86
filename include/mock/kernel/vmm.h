#pragma once

#define YUKTI_TEST_STRIP_PREFIX
#include <unittest/yukti.h>
#include <types.h>
#include <vmm.h>

DECLARE_FUNC(PTR, kvmm_memmap, VMemoryManager*, PTR, Physical const* const, SIZE,
                 VMemoryMemMapFlags, Physical* const );

void resetVMMFake();
