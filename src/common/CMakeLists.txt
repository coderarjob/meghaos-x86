set(COMMON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bitmap.c
    )

compile_lib(
    NAME common
    SOURCES ${COMMON_SOURCES}
    FLAGS ${MOS_KERNEL_GCC_FLAGS}
    INCLUDE_DIRECTORIES ${MOS_GCC_INCLUDE_DIRS}
    )
