#pragma once

#include <unittest/fake.h>
#include <cm/cm.h>

DECLARE_FUNC (void*, cm_memset, void*, U8, size_t);
DECLARE_FUNC (char*, cm_strncpy, char*, const char*, size_t);
DECLARE_FUNC (S32, syscall, U32, U32, U32, U32, U32, U32);

void resetCMFake();
