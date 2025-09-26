include(${PROJECT_SOURCE_DIR}/cmake/${ARCH}/commonflags.cmake)

set(MOS_USER_GCC_FLAGS
    ${MOS_GCC_WARN_FLAGS}
    ${MOS_GCC_FLAGS}
    -fno-inline-small-functions
)

set(MOS_USER_GCC_DEFINITIONS
    MARCH=${MARCH}
    ARCH=${ARCH}
)

set(MOS_USER_GCC_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include/cm
)

set(MOS_USER_LINKER_SCRIPT_FILE ${PROJECT_SOURCE_DIR}/src/kernel/x86/process.ld)
set(MOS_USER_APP_ENTRY_POINT proc_main)

# ----------------------------------------------------
# Zig Compiler flags
# ----------------------------------------------------
set(MOS_ZIG_BUILD_OPTIONS
    --prominent-compile-errors
)

if (MOS_BUILD_MODE STREQUAL "DEBUG")
    list(APPEND MOS_ZIG_BUILD_OPTIONS -Doptimize=Debug)
else()
    list(APPEND MOS_ZIG_BUILD_OPTIONS -Doptimize=ReleaseSafe)
endif()

# ----------------------------------------------------
# Zig Compiler Definitions
# ----------------------------------------------------
set(MOS_ZIG_BUILD_REQUIRED_DEFINES
    -DLinkerScriptPath=${MOS_USER_LINKER_SCRIPT_FILE}
    -DLibCMPath=$<TARGET_FILE_DIR:cm>
    -DCRTPath=$<TARGET_OBJECTS:crta>
    -DCInludePath=${MOS_USER_GCC_INCLUDE_DIRS}
    -DEntryPoint=${MOS_USER_APP_ENTRY_POINT}
)
