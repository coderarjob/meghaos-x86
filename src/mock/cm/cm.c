#define YUKTI_TEST_STRIP_PREFIX
#include <unittest/yukti.h>
#include <mock/cm/cm.h>

DEFINE_FUNC (void*, cm_memset, void*, U8, size_t);
DEFINE_FUNC (char*, cm_strncpy, char*, const char*, size_t);
DEFINE_FUNC (S32, syscall, OSIF_SYSCALLS, U32, U32, U32, U32, U32);

void resetCMFake()
{
    RESET_MOCK(cm_memset);
    RESET_MOCK(cm_strncpy);
    RESET_MOCK(syscall);
}
