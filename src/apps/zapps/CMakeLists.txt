find_package(Zig 0.15.1 EXACT REQUIRED)

set(ZIG_BUILD_OPTIONS
    --prominent-compile-errors
    -Doptimize=ReleaseSafe
)

set(ZIG_BUILD_DEFINES
    -DLinkerScriptPath=${MOS_USER_LINKER_SCRIPT_FILE}
    -DLibCMPath=$<TARGET_FILE_DIR:cm>
    -DCRTPath=$<TARGET_OBJECTS:crta>
    -DCInludePath=${MOS_USER_GCC_INCLUDE_DIRS}
    -DEntryPoint=${MOS_USER_APP_ENTRY_POINT}
)

add_custom_command(
    OUTPUT
        ${MOS_OBJ_DIR}/hello
        ${MOS_BIN_DIR}/hello.flt
    BYPRODUCTS
        ${CMAKE_CURRENT_LIST_DIR}/zig-out/bin/hello
        ${CMAKE_CURRENT_LIST_DIR}/zig-out/hello.flt
    COMMAND ${ZIG_EXECUTABLE} build ${ZIG_BUILD_OPTIONS} ${ZIG_BUILD_DEFINES}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/zig-out/bin/hello ${MOS_OBJ_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/zig-out/bin/hello.flt ${MOS_BIN_DIR}
    DEPENDS
        ${CMAKE_CURRENT_LIST_DIR}/hello.zig
        ${CMAKE_CURRENT_LIST_DIR}/build.zig
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

add_custom_target(
    hello
    SOURCES
        ${MOS_OBJ_DIR}/hello
        ${MOS_BIN_DIR}/hello.flt
)

add_dependencies(hello cm crta)
add_dependencies(build-all hello)
