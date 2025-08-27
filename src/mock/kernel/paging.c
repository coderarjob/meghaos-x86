#define YUKTI_TEST_STRIP_PREFIX
#include <unittest/yukti.h>
#include <mock/kernel/paging.h>

/* DEFINE_FUNC_*DONT_MOCK is used for functions which have non integer arguments. yukti.h does not
 * support mocking for such functions. */

DEFINE_FUNC_FALLBACK (void *, kpg_temporaryMap, Physical);
DEFINE_FUNC_VOID (kpg_temporaryUnmap);

void resetPagingFake()
{
    RESET_MOCK (kpg_temporaryMap);
    RESET_MOCK (kpg_temporaryUnmap);
}
