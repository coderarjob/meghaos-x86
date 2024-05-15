set(COMMON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/bitmap.c
    )

add_library(
    common
    OBJECT
    ${COMMON_SOURCES})

target_include_directories(common PRIVATE ${MOS_GCC_INCLUDE_DIRS})
target_compile_options(common PRIVATE ${MOS_GCC_FLAGS})

target_sources(kernel.elf PRIVATE $<TARGET_OBJECTS:common>)
