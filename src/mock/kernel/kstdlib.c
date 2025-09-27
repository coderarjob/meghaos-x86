#define YUKTI_TEST_STRIP_PREFIX
#include <unittest/yukti.h>
#include <mock/kernel/kstdlib.h>

DEFINE_FUNC(void *, k_memcpy, void *, const void *, size_t);
DEFINE_FUNC(void *, k_memset, void *, U8, size_t);

void resetStdLibFake(void)
{
    RESET_MOCK(k_memcpy);
    RESET_MOCK(k_memset);
}
