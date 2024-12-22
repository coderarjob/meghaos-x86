include(${PROJECT_SOURCE_DIR}/cmake/${ARCH}/kernelflags.cmake)

# ------------------------------------------------------------------------
# C Runtime Library for Applications
# Application C Runtime is analogous to the CRT in Linux, it provides prologue & epilogue code for
# application programs to run and exit gracefully.
# ------------------------------------------------------------------------
set(APPS_CRT_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/${ARCH}/crta.c
    )

compile_lib(
    NAME crta
    SOURCES ${APPS_CRT_SOURCES}
    FLAGS ${MOS_KERNEL_GCC_FLAGS}
    DEFINITIONS ${MOS_KERNEL_GCC_DEFINITIONS}
    INCLUDE_DIRECTORIES ${MOS_KERNEL_GCC_INCLUDE_DIRS}
    )
# ------------------------------------------------------------------------
# MOS C Library
# C library to talk to the OS and helpful/common functions for use by application programs.
# ------------------------------------------------------------------------
set(APPLIB_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/process.c
        ${CMAKE_CURRENT_SOURCE_DIR}/syscalls.c
        ${CMAKE_CURRENT_SOURCE_DIR}/printf.c
        ${CMAKE_CURRENT_SOURCE_DIR}/debug.c
        ${CMAKE_CURRENT_SOURCE_DIR}/cm.c
        ${CMAKE_CURRENT_SOURCE_DIR}/string.c
        ${CMAKE_CURRENT_SOURCE_DIR}/malloc.c
    )

    if (MOS_GRAPHICS_ENABLED)
        LIST(APPEND APPLIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/graphics.c)
    endif()

compile_lib(
    NAME cm
    SOURCES ${APPLIB_SOURCES}
    FLAGS ${MOS_KERNEL_GCC_FLAGS}
    DEFINITIONS ${MOS_KERNEL_GCC_DEFINITIONS}
    DEPENDS gen-build-config-header
    INCLUDE_DIRECTORIES ${MOS_KERNEL_GCC_INCLUDE_DIRS}
    STATIC_LIB
    )

target_link_libraries(cm PUBLIC gcc)
# ------------------------------------------------------------------------
