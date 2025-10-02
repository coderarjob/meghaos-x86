# ----------------------------------------------------------------
# FindZig
# Copyright (c) 2025 Arjob Mukherjee (arjobmukherjee@gmail.com)
#
# MIT License
#
# Copyright (c) 2025 Arjob Mukherjee
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# ----------------------------------------------------------------

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
