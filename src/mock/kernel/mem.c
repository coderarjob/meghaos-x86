#include <mock/kernel/mem.h>
#include <unittest/fake.h>
#include <kernel.h>

DEFINE_FUNC_3(void *, k_memcpy, IN, void *, IN, const void *, IN, size_t);
DEFINE_FUNC_3(void *, k_memset, IN, void *, IN, INT, IN, size_t);

void reset()
{
    RESET_FAKE(k_memcpy);
    RESET_FAKE(k_memset);
}
