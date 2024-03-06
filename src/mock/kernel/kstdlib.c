#include <mock/kernel/kstdlib.h>
#include <unittest/fake.h>

DEFINE_FUNC(void *, k_memcpy, void *, const void *, size_t);
DEFINE_FUNC(void *, k_memset, void *, INT, size_t);

void resetMemFake()
{
    RESET_FAKE(k_memcpy);
    RESET_FAKE(k_memset);
}
