cmake_minimum_required(VERSION 3.20)
project(MeghaOS-V2 C ASM_NASM)

set(MOS_DEBUG_LEVEL "1" CACHE STRING "Sets DEBUG_LEVEL option to this value")
set(MOS_BUILD_MODE "Debug" CACHE STRING "Debug mode")

set(MOS_BUILD_MODES "Debug" "NDebug")
set_property(CACHE MOS_BUILD_MODE PROPERTY STRINGS ${MOS_BUILD_MODES})

#---------------------------------------------------------------------------
# Validate User settable options
#---------------------------------------------------------------------------
if (NOT MOS_BUILD_MODE IN_LIST MOS_BUILD_MODES)
    message(FATAL_ERROR "${MOS_BUILD_MODE} is not a valid build mode.")
endif()

if (CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "MOS_BUILD_MODE should be used to setup build mode.")
endif()
#---------------------------------------------------------------------------

include(cmake/assemble.cmake)
include(cmake/${ARCH}/mosflags.cmake)

set(MOS_BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(MOS_LISTS_DIR ${CMAKE_BINARY_DIR}/lists)
set(MOS_DISKIMAGE_DIR ${CMAKE_BINARY_DIR}/diskimage/${ARCH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MOS_BIN_DIR})

file(MAKE_DIRECTORY ${MOS_LISTS_DIR})
file(MAKE_DIRECTORY ${MOS_DISKIMAGE_DIR})

#---------------------------------------------------------------------------
# Just for debugging. DELETE THIS WHEN FINAL
#---------------------------------------------------------------------------
add_custom_target(debug ALL
    COMMAND ${CMAKE_COMMAND} -E echo "MOS_BUILD_MODE: ${MOS_BUILD_MODE}"
    COMMAND ${CMAKE_COMMAND} -E echo "MOS_DEBUG_LEVEL: ${MOS_DEBUG_LEVEL}"
    COMMAND ${CMAKE_COMMAND} -E echo "MODE: $ENV{MODE}"
    )
#---------------------------------------------------------------------------

if(CMAKE_CROSSCOMPILING)
    #---------------------------------------------------------------------------
    # This is the top level/parent target for other sub targets for building the
    # Kernel elf file.
    #---------------------------------------------------------------------------
    add_executable(kernel.elf)
    target_link_libraries(kernel.elf PRIVATE gcc)
    target_link_options(kernel.elf PRIVATE
        -T ${MOS_KERNEL_LINKER_SCRIPT_FILE}
        ${MOS_KERNEL_LINKER_OPTIONS}
        )
    set_target_properties(kernel.elf 
        PROPERTIES LINK_DEPENDS ${MOS_KERNEL_LINKER_SCRIPT_FILE})

    add_subdirectory(src/bootloader/x86)
    add_subdirectory(src/common)
    add_subdirectory(src/kernel)
    add_subdirectory(src/kernel/x86)
    add_subdirectory(src/userland)

    add_custom_target(kernel.flt
        BYPRODUCTS ${MOS_BIN_DIR}/kernel.flt
        COMMAND ${CROSS_OBJDIR} 
                -O binary $<TARGET_FILE:kernel.elf> ${MOS_BIN_DIR}/kernel.flt
        )

    add_custom_target(mos.flp ALL
        BYPRODUCTS ${MOS_DISKIMAGE_DIR}/mos.flp
        COMMAND ${CMAKE_SOURCE_DIR}/create-floppyimg.sh ${MOS_BIN_DIR} ${MOS_DISKIMAGE_DIR}
        )

    add_dependencies(mos.flp kernel.flt boot1 boot0 proc1.flt mpdemo.flt)

    #---------------------------------------------------------------------------
    # Runs the image in qemu
    #---------------------------------------------------------------------------
    add_custom_target(run
        DEPENDS mos.flp
        COMMAND qemu-system-i386 -m 2561k -fda ${MOS_DISKIMAGE_DIR}/mos.flp
                                      -boot a
                                      -cpu 486
                                      -debugcon stdio
                                      -no-reboot
                                      -no-shutdown
                                      -d cpu_reset
        )
else()
    message(FATAL_ERROR "Not implemented")
endif()
