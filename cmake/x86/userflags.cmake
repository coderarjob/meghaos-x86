include(${PROJECT_SOURCE_DIR}/cmake/${ARCH}/commonflags.cmake)

set(MOS_USER_GCC_FLAGS
    ${MOS_GCC_WARN_FLAGS}
    ${MOS_GCC_FLAGS}
    -fno-inline-small-functions
)

set(MOS_USER_GCC_DEFINITIONS
    ${MOS_BUILD_MODE}
    MARCH=${MARCH}
    ARCH=${ARCH}
)

set(MOS_USER_GCC_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include/applib
)

set(MOS_USER_LINKER_SCRIPT_FILE ${PROJECT_SOURCE_DIR}/src/kernel/x86/process.ld)
