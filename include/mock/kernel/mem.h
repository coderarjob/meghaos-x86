#ifndef MEM_FAKE_H
#define MEM_FAKE_H

#include <unittest/fake.h>
#include <kernel.h>

DECLARE_FUNC(void *, k_memcpy, void *, const void *, size_t);
DECLARE_FUNC(void *, k_memset, void *, INT, size_t);

void resetMemFake();
#endif //MEM_FAKE_H
