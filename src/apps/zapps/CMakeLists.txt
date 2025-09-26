find_package(Zig 0.15.1 EXACT REQUIRED)

add_zig_build(
    NAME hello
    OUTPUTS hello
    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/build.zig
        ${CMAKE_CURRENT_LIST_DIR}/MosBuild.zig
        ${CMAKE_CURRENT_LIST_DIR}/hello.zig
    DEPENDS
        cm
        crta
    FLAGS ${MOS_ZIG_BUILD_OPTIONS}
    DEFINITIONS ${MOS_ZIG_BUILD_REQUIRED_DEFINES}
    ZIG_BUILD_BIN_DIR ${CMAKE_CURRENT_LIST_DIR}/zig-out/bin
    OUT_BIN_DIR ${MOS_BIN_DIR}
    OUT_OBJ_DIR ${MOS_OBJ_DIR}
)

add_dependencies(build-all hello)
