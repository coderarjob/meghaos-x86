
set(printk_mock_sources
    ${PROJECT_SOURCE_DIR}/src/mock/kernel/x86/vgadisp.c
    )

set(pmm_mock_sources
    ${PROJECT_SOURCE_DIR}/src/mock/kernel/x86/pmm.c
    )

set(kmalloc_mock_sources
    ${PROJECT_SOURCE_DIR}/src/mock/kernel/x86/memmanage.c
    )

set(salloc_mock_sources
    ${PROJECT_SOURCE_DIR}/src/mock/kernel/x86/memmanage.c
    ${PROJECT_SOURCE_DIR}/src/mock/kernel/kstdlib.c
    )
