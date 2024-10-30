# ------------------------------------------------------------------------
# CRTA
# Application C Runtime is analogous to the CRT in Linux, it provides prologue & epilogue code for
# application programs to run and exit gracefully.
# ------------------------------------------------------------------------
set(APPS_CRT_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/${ARCH}/crta.c
    )

compile_lib(
    NAME crta
    SOURCES ${APPS_CRT_SOURCES}
    FLAGS ${MOS_USER_GCC_FLAGS}
    INCLUDE_DIRECTORIES ${MOS_GCC_INCLUDE_DIRS}
    )
# ------------------------------------------------------------------------
# LIBAPP
# C library to talk to the OS and helpful/common functions for use by application programs.
# ------------------------------------------------------------------------
set(APPLIB_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/syscalls.c
    )

compile_lib(
    NAME app
    SOURCES ${APPLIB_SOURCES}
    FLAGS ${MOS_USER_GCC_FLAGS}
    INCLUDE_DIRECTORIES ${MOS_GCC_INCLUDE_DIRS}
    STATIC_LIB
    )
# ------------------------------------------------------------------------
