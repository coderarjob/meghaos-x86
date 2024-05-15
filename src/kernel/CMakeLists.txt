set(KERNEL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/disp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/kmalloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/kstdlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/pmm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/printk.c
    ${CMAKE_CURRENT_SOURCE_DIR}/salloc.c
    )

add_library(
    kernel
    OBJECT
    ${KERNEL_SOURCES})

target_include_directories(kernel PRIVATE ${MOS_GCC_INCLUDE_DIRS})
target_compile_options(kernel PRIVATE ${MOS_GCC_FLAGS})

target_sources(kernel.elf PRIVATE $<TARGET_OBJECTS:kernel>)
