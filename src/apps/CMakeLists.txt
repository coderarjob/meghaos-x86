# ---------------------------------------------------------------------------
# Program - Proc1
# ---------------------------------------------------------------------------
compile_lib(
    NAME proc1
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/proc1.c
    FLAGS ${MOS_USER_GCC_FLAGS}
    INCLUDE_DIRECTORIES ${MOS_GCC_INCLUDE_DIRS}
    )

link(
    FLATTEN
    NAME proc1.flt
    DEPENDS proc1
    FLAGS ${MOS_LINKER_OPTIONS}
    LINKER_FILE ${MOS_USER_LINKER_SCRIPT_FILE}
    LINK_LIBRARIES app
    )

# ---------------------------------------------------------------------------
# Program - MPDEMO
# ---------------------------------------------------------------------------
compile_lib(
    NAME mpdemo
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/mpdemo.c
    FLAGS ${MOS_USER_GCC_FLAGS}
    INCLUDE_DIRECTORIES ${MOS_GCC_INCLUDE_DIRS}
    )

link(
    FLATTEN
    NAME mpdemo.flt
    DEPENDS mpdemo
    FLAGS ${MOS_LINKER_OPTIONS}
    LINKER_FILE ${MOS_USER_LINKER_SCRIPT_FILE}
    LINK_LIBRARIES app
    )
