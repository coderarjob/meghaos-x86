#pragma once

#include <unittest/fake.h>
#include <paging.h>

DECLARE_FUNC (void *, kpg_temporaryMap, Physical);
DECLARE_FUNC_VOID (kpg_temporaryUnmap);

void resetPagingFake();
