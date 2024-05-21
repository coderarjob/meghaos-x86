cmake_minimum_required(VERSION 3.20)
project(MeghaOS-V2 C ASM_NASM)

set(MOS_DEBUG_LEVEL "1" CACHE STRING "Sets DEBUG_LEVEL option to this value")
set(MOS_BUILD_MODE "DEBUG" CACHE STRING "Debug mode")

set(MOS_BUILD_MODES "DEBUG" "NDEBUG")
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
include(cmake/compile_link.cmake)

set(MOS_BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(MOS_OBJ_DIR ${CMAKE_BINARY_DIR}/obj)
set(MOS_LISTS_DIR ${CMAKE_BINARY_DIR}/lists)
set(MOS_DISKIMAGE_DIR ${CMAKE_BINARY_DIR}/diskimage/${ARCH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MOS_OBJ_DIR})

set(MOS_INTERMEDIATE_EXTENSION "in")

file(MAKE_DIRECTORY ${MOS_BIN_DIR})
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
    include(cmake/${ARCH}/kernelflags.cmake)

    add_custom_target(build-all ALL)
    add_dependencies(build-all kernel.flt boot1.flt boot0.flt proc1.flt mpdemo.flt)

    #---------------------------------------------------------------------------
    # Build & link kernel and process binaries
    #---------------------------------------------------------------------------
    add_subdirectory(src/bootloader/x86)
    add_subdirectory(src/common)
    add_subdirectory(src/kernel)
    add_subdirectory(src/kernel/x86)
    add_subdirectory(src/userland)

    link(
        FLATEN
        NAME kernel.flt
        DEPENDS 
            common
            kernel
            kernel_entry
            kernel_X86
        FLAGS ${MOS_KERNEL_LINKER_OPTIONS}
        LINKER_FILE ${MOS_KERNEL_LINKER_SCRIPT_FILE}
        LINK_LIBRARIES gcc
        )

    #---------------------------------------------------------------------------
    # Create new floppy image and the copy flat binary files
    #---------------------------------------------------------------------------
    add_custom_target(mos.flp
        DEPENDS build-all
        BYPRODUCTS ${MOS_DISKIMAGE_DIR}/mos.flp
        COMMAND ${CMAKE_SOURCE_DIR}/create-floppyimg.sh ${MOS_BIN_DIR} ${MOS_DISKIMAGE_DIR}
        )

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
