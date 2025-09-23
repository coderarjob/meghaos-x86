# ----------------------------------------------------------------
# FindZig
# ----------------------------------------------------------------
# Finds the Zig compiler
#
# This module defines the following variables:
#
# ``ZIG_EXECUTABLE``
#   The full path to zig executable.
#
# ``ZIG_FOUND``
#   True if the zig executable was found.
#
# ``ZIG_VERSION_STRING``
#   The version of Zig found.
# ----------------------------------------------------------------

find_package(PackageHandleStandardArgs REQUIRED)
find_program(ZIG_EXECUTABLE NAMES zig)

if (ZIG_EXECUTABLE)
    execute_process(
        COMMAND
            ${ZIG_EXECUTABLE} version
        OUTPUT_VARIABLE
            ZIG_VERSION_STRING
        RESULT_VARIABLE
            ZIG_PROCESS_EXIT_CODE
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Zig
    REQUIRED_VARS ZIG_EXECUTABLE
    VERSION_VAR ZIG_VERSION_STRING)

mark_as_advanced(ZIG_EXECUTABLE ZIG_VERSION_STRING)
