find_package(Zig 0.15.1 EXACT REQUIRED)

set(zig_common_source_files
    ${CMAKE_CURRENT_LIST_DIR}/mos_build.zig
)

set(source_dir ${CMAKE_CURRENT_LIST_DIR}/hello)
set(name hello)
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

add_dependencies(build-all hello)
