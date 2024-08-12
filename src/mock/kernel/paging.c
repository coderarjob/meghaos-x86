#include <unittest/fake.h>
#include <mock/kernel/paging.h>

DEFINE_FUNC (void *, kpg_temporaryMap, Physical);
DEFINE_FUNC_VOID (kpg_temporaryUnmap);

void resetPagingFake()
{
    RESET_FAKE (kpg_temporaryMap);
    RESET_FAKE (kpg_temporaryUnmap);
}
