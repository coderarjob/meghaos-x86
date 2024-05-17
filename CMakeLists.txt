cmake_minimum_required(VERSION 3.10)
project(MeghaOS-V2 C ASM_NASM)

include(cmake/assemble.cmake)

set(MOS_BIN_DIR ${CMAKE_BINARY_DIR}/bin)
set(MOS_LISTS_DIR ${CMAKE_BINARY_DIR}/lists)
set(MOS_DISKIMAGE_DIR ${CMAKE_BINARY_DIR}/diskimage/${ARCH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MOS_BIN_DIR})

file(MAKE_DIRECTORY ${MOS_LISTS_DIR})
file(MAKE_DIRECTORY ${MOS_DISKIMAGE_DIR})

if(CMAKE_CROSSCOMPILING)
    #---------------------------------------------------------------------------
    # This is the top level/parent target for other sub targets for building the
    # Kernel elf file.
    #---------------------------------------------------------------------------
    add_executable(kernel.elf)
    target_link_libraries(kernel.elf PRIVATE gcc)
    target_link_options(kernel.elf PRIVATE
        -T ${MOS_LINKER_SCRIPT_FILE}
        ${MOS_LINKER_OPTIONS}
        )
    set_target_properties(kernel.elf 
        PROPERTIES LINK_DEPENDS ${MOS_LINKER_SCRIPT_FILE})

    add_custom_command(
        TARGET kernel.elf
        POST_BUILD
        BYPRODUCTS 
            ${MOS_BIN_DIR}/kernel.flt
            ${MOS_DISKIMAGE_DIR}/mos.flp
        COMMAND ${CROSS_OBJDIR} 
                -O binary $<TARGET_FILE:kernel.elf> ${MOS_BIN_DIR}/kernel.flt
        COMMAND ${CMAKE_SOURCE_DIR}/create-floppyimg.sh ${MOS_BIN_DIR} ${MOS_DISKIMAGE_DIR}
        )

    add_subdirectory(src/bootloader/x86)
    add_subdirectory(src/common)
    add_subdirectory(src/kernel)
    add_subdirectory(src/kernel/x86)
else()
    message(FATAL_ERROR "Not implemented")
endif()
