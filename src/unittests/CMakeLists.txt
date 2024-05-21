
include(${PROJECT_SOURCE_DIR}/src/mock/mockfortests.cmake)

set(UNITTESTS
    c99_conformance
    printk
    kstdlib
    bitmap
    pmm
    utils
    paging
    intrusive_list
    kmalloc
    salloc
    intrusive_queue
    )

set(c99_conformance_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/c99_conformance_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    )

set(kstdlib_sources
    ${PROJECT_SOURCE_DIR}/src/kernel/kstdlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/kstdlib_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    )

set(bitmap_sources
    ${PROJECT_SOURCE_DIR}/src/common/bitmap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/bitmap_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    ${CMAKE_CURRENT_SOURCE_DIR}/common.c
    )

set(utils_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/utils_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    )

set(paging_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/paging_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    )

set(intrusive_list_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/intrusive_list_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    )

set(intrusive_queue_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/intrusive_queue_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    )

set(printk_sources
    ${PROJECT_SOURCE_DIR}/src/kernel/printk.c
    ${CMAKE_CURRENT_SOURCE_DIR}/printk_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    ${printk_mock_sources}
    )

set(pmm_sources
    ${PROJECT_SOURCE_DIR}/src/kernel/pmm.c
    ${PROJECT_SOURCE_DIR}/src/common/bitmap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pmm_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    ${pmm_mock_sources}
    )

set(kmalloc_sources
    ${PROJECT_SOURCE_DIR}/src/kernel/kmalloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/kmalloc_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    ${kmalloc_mock_sources}
    )

set(salloc_sources
    ${PROJECT_SOURCE_DIR}/src/kernel/salloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/salloc_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/common.c
    ${CMAKE_CURRENT_SOURCE_DIR}/unittest.c
    ${salloc_mock_sources}
    )

foreach(test ${UNITTESTS})
    compile_lib(
        NAME ${test}
        SOURCES ${${test}_sources}
        FLAGS ${MOS_UNITTESTS_GCC_FLAGS}
        INCLUDE_DIRECTORIES ${MOS_GCC_INCLUDE_DIRS}
        )

    link(
        NO_LIST
        NAME ${test}_test
        DEPENDS ${test}
        FLAGS ${MOS_UNITEST_LINKER_OPTIONS}
        LINK_LIBRARIES gcov m
        )
    add_dependencies(build-all ${test}_test)
endforeach()
