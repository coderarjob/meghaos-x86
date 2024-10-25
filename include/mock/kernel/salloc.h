#include <unittest/fake.h>
#include <memmanage.h>
DECLARE_FUNC(void*, ksalloc,UINT);
DECLARE_FUNC(void*, kscalloc,UINT);
void reset_sallocFake();
