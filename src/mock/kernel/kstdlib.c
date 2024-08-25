#include <mock/kernel/kstdlib.h>
#include <unittest/fake.h>

DEFINE_FUNC(void *, k_memcpy, void *, const void *, size_t);
DEFINE_FUNC(void *, k_memset, void *, U8, size_t);

void resetStdLibFake()
{
    RESET_FAKE(k_memcpy);
    RESET_FAKE(k_memset);
}
