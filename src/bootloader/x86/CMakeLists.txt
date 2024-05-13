set(BOOT0_FILE ${MOS_BIN_DIR}/boot0.flt)
set(BOOT1_FILE ${MOS_BIN_DIR}/boot1.flt)

ASSEMBLE(
    TARGET boot0
    SOURCE boot0/boot.s
    COPYTO ${BOOT0_FILE}
    LISTDIR ${MOS_LISTS_DIR}
    FLAGS -O0 -f bin
    INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/boot0
    ${MOS_ASM_INCLUDE_DIRS}
    )

ASSEMBLE(
    TARGET boot1
    SOURCE boot1/boot1.s
    COPYTO ${BOOT1_FILE}
    LISTDIR ${MOS_LISTS_DIR}
    FLAGS -O0 -f bin
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/boot1/
        ${MOS_ASM_INCLUDE_DIRS}
    )

add_custom_target(
    bootloader ALL
    DEPENDS
        ${BOOT0_FILE}
        ${BOOT1_FILE}
        ${MOS_LISTS_DIR}/boot0.lst
        ${MOS_LISTS_DIR}/boot1.lst
    )
