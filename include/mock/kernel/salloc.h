#define YUKTI_TEST_STRIP_PREFIX
#include <unittest/yukti.h>
#include <memmanage.h>
DECLARE_FUNC(void*, ksalloc,UINT);
DECLARE_FUNC(void*, kscalloc,UINT);
void reset_sallocFake();
