
test(
    NAME pmm_x86_test
    DEPENDENT_FOR build-all
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/kernel/x86/pmm.c
        ${CMAKE_CURRENT_SOURCE_DIR}/pmm_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../unittest.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../common.c
        # Mock sources
        ${PROJECT_SOURCE_DIR}/src/mock/kernel/kstdlib.c
        ${PROJECT_SOURCE_DIR}/src/mock/kernel/bitmap.c
        ${PROJECT_SOURCE_DIR}/src/mock/kernel/pmm.c
        ${PROJECT_SOURCE_DIR}/src/mock/kernel/x86/boot.c
    )

test(
    NAME paging_x86_test
    DEPENDENT_FOR build-all
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/kernel/x86/paging.c
        ${CMAKE_CURRENT_SOURCE_DIR}/paging_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../unittest.c
        ${CMAKE_CURRENT_SOURCE_DIR}/../common.c
        # Mock sources
        ${PROJECT_SOURCE_DIR}/src/mock/kernel/kstdlib.c
        ${PROJECT_SOURCE_DIR}/src/mock/kernel/pmm.c
        ${PROJECT_SOURCE_DIR}/src/mock/kernel/x86/paging.c
    )
