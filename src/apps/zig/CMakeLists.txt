find_package(Zig 0.15.1 EXACT REQUIRED)

set(zig_common_source_files
    ${CMAKE_CURRENT_LIST_DIR}/mos_build.zig
    ${CMAKE_CURRENT_LIST_DIR}/zm/src/debug.zig
    ${CMAKE_CURRENT_LIST_DIR}/zm/src/root.zig
    ${CMAKE_CURRENT_LIST_DIR}/zm/src/process.zig
    ${CMAKE_CURRENT_LIST_DIR}/zm/build.zig
    ${CMAKE_CURRENT_LIST_DIR}/zm/build.zig.zon
)

set(source_dir ${CMAKE_CURRENT_LIST_DIR}/zello)
set(name zello)
add_zig_build(
    NAME ${name}
    SOURCES
        ${zig_common_source_files}
        ${source_dir}/main.zig
        ${source_dir}/build.zig
        ${source_dir}/build.zig.zon
    DEPENDS
        cm
        crta
    FLAGS ${MOS_ZIG_BUILD_OPTIONS}
    DEFINITIONS ${MOS_ZIG_BUILD_REQUIRED_DEFINES}
    OUTPUT ${source_dir}/zig-out/bin/${name}          COPY_TO ${MOS_OBJ_DIR}
    OUTPUT ${source_dir}/zig-out/bin/${name}.flt      COPY_TO ${MOS_BIN_DIR}
    WORKING_DIRECTORY ${source_dir}
)

add_dependencies(build-all zello)
