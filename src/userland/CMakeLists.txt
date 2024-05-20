set(MOS_PROCESS_LINKER_SCRIPT_FILE ${PROJECT_SOURCE_DIR}/build/process.ld)
# ---------------------------------------------------------------------------
# Program - Proc1
# ---------------------------------------------------------------------------
set(PROC1_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/proc1.c)
add_executable(
    proc1
    ${PROC1_SOURCES})

target_link_options(proc1 PRIVATE
    -T ${MOS_PROCESS_LINKER_SCRIPT_FILE}
    ${MOS_KERNEL_LINKER_OPTIONS}
    )
set_target_properties(proc1
    PROPERTIES LINK_DEPENDS ${MOS_PROCESS_LINKER_SCRIPT_FILE})

target_include_directories(proc1 PRIVATE ${MOS_GCC_INCLUDE_DIRS})
target_compile_options(proc1 PRIVATE ${MOS_GCC_FLAGS})

add_custom_target(proc1.flt
    BYPRODUCTS ${MOS_BIN_DIR}/proc1.flt
    COMMAND ${CROSS_OBJDIR} 
    -O binary $<TARGET_FILE:proc1> ${MOS_BIN_DIR}/proc1.flt
    )

add_dependencies(proc1.flt proc1)

# ---------------------------------------------------------------------------
# Program - MPDEMO
# ---------------------------------------------------------------------------
set(MPDEMO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/mpdemo.c)
add_executable(
    mpdemo
    ${MPDEMO_SOURCES}
    )

target_link_options(mpdemo PRIVATE
    -T ${MOS_PROCESS_LINKER_SCRIPT_FILE}
    ${MOS_KERNEL_LINKER_OPTIONS}
    )

set_target_properties(mpdemo
    PROPERTIES LINK_DEPENDS ${MOS_PROCESS_LINKER_SCRIPT_FILE})

target_include_directories(mpdemo PRIVATE ${MOS_GCC_INCLUDE_DIRS})
target_compile_options(mpdemo PRIVATE ${MOS_GCC_FLAGS})

add_custom_target(mpdemo.flt
    BYPRODUCTS ${MOS_BIN_DIR}/mpdemo.flt
    COMMAND ${CROSS_OBJDIR} 
    -O binary $<TARGET_FILE:mpdemo> ${MOS_BIN_DIR}/mpdemo.flt
    )
add_dependencies(mpdemo.flt mpdemo)
