
include(${PROJECT_SOURCE_DIR}/src/mock/mockfortests_${ARCH}.cmake)

set(COMMON_KERNEL_UT_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/unittests/unittest.c
    ${CMAKE_CURRENT_SOURCE_DIR}/common.c
)

#---------------------------------------------------------------------------
# Build Arch dependent kernel tests
#---------------------------------------------------------------------------
add_subdirectory(${ARCH})

#---------------------------------------------------------------------------
# Build Arch independent kernel tests
#---------------------------------------------------------------------------
test(
    NAME c99_conformance_test
    DEPENDENT_FOR build-all
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/c99_conformance_test.c
        ${COMMON_KERNEL_UT_SOURCE_FILES}
    )

test(
    NAME kstdlib_test
    DEPENDENT_FOR build-all
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/kernel/kstdlib.c
        ${CMAKE_CURRENT_SOURCE_DIR}/kstdlib_test.c
        ${kstdlib_mock_sources}
        ${COMMON_KERNEL_UT_SOURCE_FILES}
    )

test(
    NAME printk_test
    DEPENDENT_FOR build-all
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/kernel/printk.c
        ${CMAKE_CURRENT_SOURCE_DIR}/printk_test.c
        ${COMMON_KERNEL_UT_SOURCE_FILES}
    )

test(
    NAME bitmap_test
    DEPENDENT_FOR build-all
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/kernel/bitmap.c
        ${CMAKE_CURRENT_SOURCE_DIR}/bitmap_test.c
        ${COMMON_KERNEL_UT_SOURCE_FILES}
    )

test(
    NAME utils_test
    DEPENDENT_FOR build-all
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/utils_test.c
        ${COMMON_KERNEL_UT_SOURCE_FILES}
    )

test(
    NAME paging_test
    DEPENDENT_FOR build-all
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/paging_test.c
        ${COMMON_KERNEL_UT_SOURCE_FILES}
    )

test(
    NAME intrusive_list_test
    DEPENDENT_FOR build-all
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/intrusive_list_test.c
        ${COMMON_KERNEL_UT_SOURCE_FILES}
    )

test(
    NAME intrusive_queue_test
    DEPENDENT_FOR build-all
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/intrusive_queue_test.c
        ${COMMON_KERNEL_UT_SOURCE_FILES}
    )

test(
    NAME pmm_test
    DEPENDENT_FOR build-all
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/kernel/pmm.c
        ${PROJECT_SOURCE_DIR}/src/kernel/bitmap.c
        ${CMAKE_CURRENT_SOURCE_DIR}/pmm_test.c
        ${pmm_mock_sources}
        ${COMMON_KERNEL_UT_SOURCE_FILES}
    )

test(
    NAME kmalloc_test
    DEPENDENT_FOR build-all
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/kernel/kmalloc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/kmalloc_test.c
        ${kmalloc_mock_sources}
        ${COMMON_KERNEL_UT_SOURCE_FILES}
    )

test(
    NAME salloc_test
    DEPENDENT_FOR build-all
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/kernel/salloc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/salloc_test.c
        ${salloc_mock_sources}
        ${COMMON_KERNEL_UT_SOURCE_FILES}
    )

test(
    NAME handle_test
    DEPENDENT_FOR build-all
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/kernel/handle.c
        ${CMAKE_CURRENT_SOURCE_DIR}/handle_test.c
        ${handles_mock_sources}
        ${COMMON_KERNEL_UT_SOURCE_FILES}
    )
#---------------------------------------------------------------------------
