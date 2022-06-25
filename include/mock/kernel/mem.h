#ifndef MEM_FAKE_H
#define MEM_FAKE_H

#include <unittest/fake.h>
#include <kernel.h>

DECLARE_FUNC_3(void *, k_memcpy, IN, void *, IN, const void *, IN, size_t);
DECLARE_FUNC_3(void *, k_memset, IN, void *, IN, INT, IN, size_t);

#endif //MEM_FAKE_H
