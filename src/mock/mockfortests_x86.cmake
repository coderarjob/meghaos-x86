
set(pmm_mock_sources
    ${PROJECT_SOURCE_DIR}/src/mock/kernel/x86/pmm.c
    )

set(kmalloc_mock_sources
    ${PROJECT_SOURCE_DIR}/src/mock/kernel/vmm.c
    ${PROJECT_SOURCE_DIR}/src/mock/kernel/kstdlib.c
    )

set(salloc_mock_sources
    ${PROJECT_SOURCE_DIR}/src/mock/kernel/kstdlib.c
    )

set(kstdlib_mock_sources
    ${PROJECT_SOURCE_DIR}/src/mock/kernel/paging.c
    )

set(handles_mock_sources
    ${PROJECT_SOURCE_DIR}/src/mock/kernel/salloc.c
    )

set(cm_malloc_mock_sources
    ${PROJECT_SOURCE_DIR}/src/mock/cm/cm.c
    )
