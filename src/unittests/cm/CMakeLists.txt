include(${PROJECT_SOURCE_DIR}/src/mock/mockfortests_${ARCH}.cmake)

test(
    NAME cm_printf_test
    DEPENDENT_FOR build-all
    DEFINITIONS LIBCM
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/cm/printf.c
        ${PROJECT_SOURCE_DIR}/src/unittests/unittest.c
        ${CMAKE_CURRENT_SOURCE_DIR}/printf_test.c
    )

test(
    NAME cm_string_test
    DEPENDENT_FOR build-all
    DEFINITIONS LIBCM
    SOURCES
        ${PROJECT_SOURCE_DIR}/src/cm/string.c
        ${PROJECT_SOURCE_DIR}/src/unittests/unittest.c
        ${CMAKE_CURRENT_SOURCE_DIR}/string_test.c
    )
