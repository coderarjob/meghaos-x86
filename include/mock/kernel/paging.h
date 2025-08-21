#pragma once

#define YUKTI_TEST_STRIP_PREFIX
#include <unittest/yukti.h>
#include <paging.h>

DECLARE_FUNC (void *, kpg_temporaryMap, Physical);
DECLARE_FUNC_VOID (kpg_temporaryUnmap);

void resetPagingFake();
