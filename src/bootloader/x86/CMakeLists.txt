assemble_and_copy_bin(
    NAME boot0.flt
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/boot0/boot.s
    FLAGS -O0 -f bin
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/boot0
        ${MOS_ASM_INCLUDE_DIRS}
    )

assemble_and_copy_bin(
    NAME boot1.flt
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/boot1/boot1.s
    FLAGS -O0 -f bin
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/boot1
        ${MOS_ASM_INCLUDE_DIRS}
    )
