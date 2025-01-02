#include <mock/cm/cm.h>
#include <unittest/fake.h>

DEFINE_FUNC (void*, cm_memset, void*, U8, size_t);
DEFINE_FUNC (char*, cm_strncpy, char*, const char*, size_t);
DEFINE_FUNC (S32, syscall, U32, U32, U32, U32, U32, U32);

void resetCMFake()
{
    RESET_FAKE(cm_memset);
    RESET_FAKE(cm_strncpy);
    RESET_FAKE(syscall);
}
