set(KERNEL_X86_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/boot.c
    ${CMAKE_CURRENT_SOURCE_DIR}/gdt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/idt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/interrupts.c
    ${CMAKE_CURRENT_SOURCE_DIR}/kdebug.c
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/memmanage.c
    ${CMAKE_CURRENT_SOURCE_DIR}/paging.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pmm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/process.c
    ${CMAKE_CURRENT_SOURCE_DIR}/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tss.c
    ${CMAKE_CURRENT_SOURCE_DIR}/vgadisp.c
    )

add_library(
    kernel_X86
    STATIC
    ${KERNEL_X86_SOURCES})

target_include_directories(kernel_X86 PRIVATE ${MOS_GCC_INCLUDE_DIRS})
target_compile_options(kernel_X86 PRIVATE ${MOS_GCC_FLAGS})

add_library(kernel_entry
    STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/entry.s
    )

target_include_directories(kernel_entry PRIVATE ${MOS_ASM_INCLUDE_DIRS})
target_compile_options(kernel_entry PRIVATE -O0 -f elf)
