find_package(Zig 0.15.1 EXACT REQUIRED)

set(ZIG_COMMON_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/mos_build.zig
)

add_zig_build(
    NAME hello
    SOURCES
        ${ZIG_COMMON_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/hello/main.zig
        ${CMAKE_CURRENT_LIST_DIR}/hello/build.zig
    DEPENDS
        cm
        crta
    FLAGS ${MOS_ZIG_BUILD_OPTIONS}
    DEFINITIONS ${MOS_ZIG_BUILD_REQUIRED_DEFINES}
    OUTPUT ${CMAKE_CURRENT_LIST_DIR}/hello/zig-out/bin/hello     COPY_TO ${MOS_OBJ_DIR}
    OUTPUT ${CMAKE_CURRENT_LIST_DIR}/hello/zig-out/bin/hello.flt COPY_TO ${MOS_BIN_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/hello
)

add_dependencies(build-all hello)
