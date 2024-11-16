set(MOS_UNITTESTS_GCC_WARN_FLAGS
    -wall
    -wextra
    )

set(MOS_UNITTESTS_GCC_FLAGS
    ${MOS_UNITTESTS_GCC_FLAGS}
    -std=c99
    -g
    -m32
    -march=i386
    -masm=intel
    --coverage
    -DUNITTEST
    -DARCH=${ARCH}
    -DMARCH=${MARCH}
    -D${MOS_BUILD_MODE}
    )

if (MOS_BUILD_MODE STREQUAL "DEBUG")
    set(MOS_UNITTESTS_GCC_FLAGS ${MOS_UNITTESTS_GCC_FLAGS} -g)
endif()

set(MOS_UNITEST_LINKER_OPTIONS
    -m32
    )

set(MOS_ASM_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include/asm
    ${PROJECT_SOURCE_DIR}/include/${ARCH}/asm
    )

set(MOS_GCC_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include
    )
