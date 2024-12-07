include(${PROJECT_SOURCE_DIR}/cmake/${ARCH}/userflags.cmake)

# ---------------------------------------------------------------------------
# Program - Proc1
# ---------------------------------------------------------------------------
compile_lib(
    NAME proc1
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/proc1.c
    FLAGS ${MOS_USER_GCC_FLAGS}
    DEFINITIONS ${MOS_USER_GCC_DEFINITIONS}
    INCLUDE_DIRECTORIES ${MOS_USER_GCC_INCLUDE_DIRS}
    )

link(
    FLATTEN
    NAME proc1.flt
    DEPENDS proc1 crta
    FLAGS ${MOS_LINKER_OPTIONS}
    LINKER_FILE ${MOS_USER_LINKER_SCRIPT_FILE}
    LINK_LIBRARIES cm
    )

# ---------------------------------------------------------------------------
# Program - MPDEMO
# ---------------------------------------------------------------------------
compile_lib(
    NAME mpdemo
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/mpdemo.c
    FLAGS ${MOS_USER_GCC_FLAGS}
    DEFINITIONS ${MOS_USER_GCC_DEFINITIONS}
    INCLUDE_DIRECTORIES ${MOS_USER_GCC_INCLUDE_DIRS}
    )

link(
    FLATTEN
    NAME mpdemo.flt
    DEPENDS mpdemo crta
    FLAGS ${MOS_LINKER_OPTIONS}
    LINKER_FILE ${MOS_USER_LINKER_SCRIPT_FILE}
    LINK_LIBRARIES cm
    )

# ---------------------------------------------------------------------------
# Program - GUI0
# ---------------------------------------------------------------------------
compile_lib(
    NAME gui0
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/gui0.c
    FLAGS ${MOS_USER_GCC_FLAGS}
    DEFINITIONS ${MOS_USER_GCC_DEFINITIONS}
                GRAPHICS_BPP=${MOS_GRAPHICS_BPP}
    INCLUDE_DIRECTORIES ${MOS_USER_GCC_INCLUDE_DIRS}
    )

link(
    FLATTEN
    NAME gui0.flt
    DEPENDS gui0 crta
    FLAGS ${MOS_LINKER_OPTIONS}
    LINKER_FILE ${MOS_USER_LINKER_SCRIPT_FILE}
    LINK_LIBRARIES cm
    )

# ---------------------------------------------------------------------------
# Program - INIT
# ---------------------------------------------------------------------------
compile_lib(
    NAME init
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/init.c
    FLAGS ${MOS_USER_GCC_FLAGS}
    DEFINITIONS ${MOS_USER_GCC_DEFINITIONS} DEBUG PORT_E9_ENABLED
    INCLUDE_DIRECTORIES ${MOS_USER_GCC_INCLUDE_DIRS}
    )

link(
    FLATTEN
    NAME init.flt
    DEPENDS init crta
    FLAGS ${MOS_LINKER_OPTIONS}
    LINKER_FILE ${MOS_USER_LINKER_SCRIPT_FILE}
    LINK_LIBRARIES cm
    )

#---------------------------------------------------------------------------
# Add to the global 'build-all' target
#---------------------------------------------------------------------------
add_dependencies(build-all init.flt)

if (MOS_GRAPHICS_ENABLED)
    add_dependencies(build-all gui0.flt)
else()
    add_dependencies(build-all proc1.flt mpdemo.flt)
endif()
