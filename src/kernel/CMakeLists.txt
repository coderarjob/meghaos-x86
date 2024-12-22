include(${PROJECT_SOURCE_DIR}/cmake/${ARCH}/kernelflags.cmake)

#---------------------------------------------------------------------------
# Generate header file
#---------------------------------------------------------------------------
add_custom_target(gen-build-config-header
                  BYPRODUCTS ${MOS_BUILD_CONFIG_HEADER_FILE}
                  COMMAND ${CMAKE_COMMAND}
                          -DGenHeader=1
                          -DDefinitions="${MOS_KERNEL_GCC_DEFINITIONS}"
                          -DHeaderFileDestinationPath=${MOS_BUILD_CONFIG_HEADER_FILE}
                          -DHeaderFileTemplatePath=${MOS_BUILD_CONFIG_HEADER_TEMPLATE_FILE}
                          -P ${PROJECT_SOURCE_DIR}/cmake/GenConfigHeaderFile.cmake
)

#---------------------------------------------------------------------------
# Build Arch independent kernel
#---------------------------------------------------------------------------
set(KERNEL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/kpanic.c
    ${CMAKE_CURRENT_SOURCE_DIR}/kmalloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/kstdlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pmm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/printk.c
    ${CMAKE_CURRENT_SOURCE_DIR}/salloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vmm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/handle.c
    ${CMAKE_CURRENT_SOURCE_DIR}/bitmap.c
    )

    if (MOS_GRAPHICS_ENABLED)
        LIST(APPEND KERNEL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/kgraphics.c)
        LIST(APPEND KERNEL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/compositor.c)
    endif()

    if (MOS_BUILD_MODE STREQUAL "DEBUG")
        LIST(APPEND KERNEL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/printk.c)
        if (MOS_PORT_E9_ENABLED)
            LIST(APPEND KERNEL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/kdebug.c)
        endif()
    endif()

compile_lib(
    NAME kernel
    SOURCES ${KERNEL_SOURCES}
    FLAGS ${MOS_KERNEL_GCC_FLAGS}
    DEFINITIONS ${MOS_KERNEL_GCC_DEFINITIONS}
    INCLUDE_DIRECTORIES ${MOS_KERNEL_GCC_INCLUDE_DIRS}
    )

#---------------------------------------------------------------------------
# Build Arch dependent kernel & drivers
#---------------------------------------------------------------------------
add_subdirectory(${ARCH})
add_subdirectory(drivers/${ARCH}/${MARCH})

set(KERNEL_RESOURCE_FILES)

#---------------------------------------------------------------------------
# Link the kernel
#---------------------------------------------------------------------------
if (MOS_GRAPHICS_ENABLED)
    list (APPEND KERNEL_RESOURCE_FILES ${PROJECT_SOURCE_DIR}/docs/images/mos.rbm)
endif()

link(
    FLATTEN
    NAME kernel.flt
    DEPENDS
        kernel
        kernel_entry
        kernel_X86
        kernel_drivers
    RESOURCES ${KERNEL_RESOURCE_FILES}
    FLAGS ${MOS_LINKER_OPTIONS}
    LINKER_FILE ${MOS_KERNEL_LINKER_SCRIPT_FILE}
    LINK_LIBRARIES gcc
    )

#---------------------------------------------------------------------------
# Add to the global 'build-all' target
#---------------------------------------------------------------------------
add_dependencies(build-all kernel.flt)
