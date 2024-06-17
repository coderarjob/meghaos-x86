#ifndef MEM_FAKE_H
#define MEM_FAKE_H

#include <unittest/fake.h>
#include <types.h>

DECLARE_FUNC(void *, k_memcpy, void *, const void *, size_t);
DECLARE_FUNC(void *, k_memset, void *, U8, size_t);

void resetStdLibFake();
#endif //MEM_FAKE_H
