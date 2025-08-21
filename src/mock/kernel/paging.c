/* YUKTI_TEST_NO_MUST_CALL is required because MUST_CALL_* only works for functions with integer
 * parameters. Here kpg_temporaryMap takes Physical argument which causes build failure if
 * YUKTI_TEST_NO_MUST_CALL is not defined.*/
#define YUKTI_TEST_STRIP_PREFIX
#define YUKTI_TEST_NO_MUST_CALL
#include <unittest/yukti.h>
#include <mock/kernel/paging.h>

DEFINE_FUNC (void *, kpg_temporaryMap, Physical);
DEFINE_FUNC_VOID (kpg_temporaryUnmap);

void resetPagingFake()
{
    RESET_MOCK (kpg_temporaryMap);
    RESET_MOCK (kpg_temporaryUnmap);
}
