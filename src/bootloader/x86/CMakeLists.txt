include(${PROJECT_SOURCE_DIR}/cmake/${ARCH}/kernelflags.cmake)

assemble_and_copy_bin(
    NAME boot0.flt
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/boot0/boot.s
    FLAGS ${MOS_KERNEL_NASM_BIN_MODE_FLAGS}
    DEFINITIONS ${MOS_KERNEL_GCC_DEFINITIONS}
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/boot0
        ${MOS_KERNEL_ASM_INCLUDE_DIRS}
    )

assemble_and_copy_bin(
    NAME boot1.flt
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/boot1/boot1.s
    FLAGS ${MOS_KERNEL_NASM_BIN_MODE_FLAGS}
    DEFINITIONS ${MOS_KERNEL_GCC_DEFINITIONS}
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/boot1
        ${MOS_KERNEL_ASM_INCLUDE_DIRS}
    )

#---------------------------------------------------------------------------
# Add to the global 'build-all' target
#---------------------------------------------------------------------------
add_dependencies(build-all boot0.flt boot1.flt)
