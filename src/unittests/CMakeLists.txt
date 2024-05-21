
include(${PROJECT_SOURCE_DIR}/src/mock/mockfortests_${ARCH}.cmake)

test(
    NAME c99_conformance
    DEPENDENT_FOR build-all
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/c99_conformance_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    )

test(
    NAME kstdlib
    DEPENDENT_FOR build-all
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/kernel/kstdlib.c
        ${CMAKE_CURRENT_SOURCE_DIR}/kstdlib_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    )

test(
    NAME printk
    DEPENDENT_FOR build-all
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/kernel/printk.c
        ${CMAKE_CURRENT_SOURCE_DIR}/printk_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/common.c
        ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
        ${printk_mock_sources}
    )

test(
    NAME bitmap
    DEPENDENT_FOR build-all
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/common/bitmap.c
        ${CMAKE_CURRENT_SOURCE_DIR}/bitmap_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
        ${CMAKE_CURRENT_SOURCE_DIR}/common.c
    )

test(
    NAME utils
    DEPENDENT_FOR build-all
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/utils_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    )

test(
    NAME paging
    DEPENDENT_FOR build-all
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/paging_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    )

test(
    NAME intrusive_list
    DEPENDENT_FOR build-all
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/intrusive_list_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    )

test(
    NAME intrusive_queue
    DEPENDENT_FOR build-all
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/intrusive_queue_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    )

test(
    NAME pmm
    DEPENDENT_FOR build-all
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/kernel/pmm.c
        ${PROJECT_SOURCE_DIR}/src/common/bitmap.c
        ${CMAKE_CURRENT_SOURCE_DIR}/pmm_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/common.c
        ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
        ${pmm_mock_sources}
    )

test(
    NAME kmalloc
    DEPENDENT_FOR build-all
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/kernel/kmalloc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/kmalloc_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/common.c
        ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
        ${kmalloc_mock_sources}
    )

test(
    NAME salloc
    DEPENDENT_FOR build-all
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/kernel/salloc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/salloc_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/common.c
        ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
        ${salloc_mock_sources}
    )
