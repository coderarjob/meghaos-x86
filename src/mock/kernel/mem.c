#include <mock/kernel/mem.h>
#include <unittest/fake.h>
#include <kernel.h>

DEFINE_FUNC_3(void *, k_memcpy, void *, const void *, size_t);
DEFINE_FUNC_3(void *, k_memset, void *, INT, size_t);

void resetMemFake()
{
    RESET_FAKE(k_memcpy);
    RESET_FAKE(k_memset);
}
