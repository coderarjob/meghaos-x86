include(${PROJECT_SOURCE_DIR}/cmake/${ARCH}/kernelflags.cmake)

#---------------------------------------------------------------------------
# Generate header file
#---------------------------------------------------------------------------
set(LIBCM_BUILD_CONFIG_HEADER_FILE          ${PROJECT_SOURCE_DIR}/include/cm/cmbuild.h)
set(LIBCM_BUILD_CONFIG_HEADER_TEMPLATE_FILE ${PROJECT_SOURCE_DIR}/include/cm/cmbuild.h.in)

add_custom_target(gen-build-config-header
                  BYPRODUCTS ${LIBCM_BUILD_CONFIG_HEADER_FILE}
                  COMMAND ${CMAKE_COMMAND}
                          -DGenHeader=1
                          -DDefinitions="${MOS_KERNEL_GCC_DEFINITIONS}"
                          -DHeaderFileDestinationPath=${LIBCM_BUILD_CONFIG_HEADER_FILE}
                          -DHeaderFileTemplatePath=${LIBCM_BUILD_CONFIG_HEADER_TEMPLATE_FILE}
                          -P ${PROJECT_SOURCE_DIR}/cmake/GenConfigHeaderFile.cmake
)

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
