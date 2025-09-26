# ==================================================================================================
# add_zig_build (NAME name
#              SOURCES <source> [<source> ...]
#              OUTPUT  <zig build output file>    COPY_TO <copy output to this directory>
#              [OUTPUT <zig build output file>    COPY_TO <copy output to this directory>]
#              [DEPENDS <target> ...]
#              [FLAGS <flag> ...]
#              [DEFINITIONS <compiler macros> ...]
#              [WORKING_DIRECTORY <dir>])
#
# Invokes `zig build` command to build output artifacts. These are then copied to some destination
# directory. 
#
# OUTPUT
# A file that the zig build generates. A matching `COPY_TO <dir>` will copy this file to `<dir>`
# directory.
# 
# COPY_TO
# Destination directory where the corresponding `OUTPUT` <file> will be copied to. Since each OUTPUT
# and COPY_TO arguments are linked, their count must match.
#
# SOURCES
# Zig source files which this build depends on. Zig build is triggered if any of these files is
# latest.
#
# DEPENDS
# Target which must be build before compiling the source files.
#
# FLAGS
# Compiler flags/options.
#
# DEFINITIONS
# Definitions/macros which are passed to the compiler.
#
# WORKING_DIRECTORY
# Changes to the provided directory before `zig build` is run.
# ==================================================================================================
function(add_zig_build)
    set(oneValueArgs NAME WORKING_DIRECTORY)
    set(multiValueArgs OUTPUT COPY_TO DEPENDS SOURCES FLAGS DEFINITIONS)
    set(options)

    cmake_parse_arguments(PARSE_ARGV 0 ZBUILD "${options}" "${oneValueArgs}" "${multiValueArgs}")

    # -------------------------------------------------------------------------------------------
    # Check validity
    # -------------------------------------------------------------------------------------------
    if (ZBUILD_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Invalid option: ${ZBUILD_UNPARSED_ARGUMENTS}")
    endif()

    if (NOT ZBUILD_NAME)
        message(FATAL_ERROR "Name must be given.")
    endif()

    list(LENGTH ZBUILD_SOURCES SOURCE_LEN)
    if (NOT SOURCE_LEN)
        message(FATAL_ERROR "There must be at least one source file.")
    endif()

    list(LENGTH ZBUILD_OUTPUT OUTPUT_LEN)
    if (NOT OUTPUT_LEN)
        message(FATAL_ERROR "There must be at least one output file.")
    endif()

    list(LENGTH ZBUILD_COPY_TO COPY_TO_LEN)
    if (NOT COPY_TO_LEN EQUAL OUTPUT_LEN)
        message(
            FATAL_ERROR
            "There must be as many COPY_TO arguments as there are OUTPUT arguments.")
    endif()

    if (NOT ZBUILD_WORKING_DIRECTORY)
        set(ZBUILD_WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
    endif()
    # -------------------------------------------------------------------------------------------
    # Command to build the executable and copy to destination folders
    # -------------------------------------------------------------------------------------------
    add_custom_command(
        OUTPUT ${ZBUILD_OUTPUT}
        COMMAND ${ZIG_EXECUTABLE} build ${ZBUILD_FLAGS} ${ZBUILD_DEFINITIONS}
        DEPENDS ${ZBUILD_SOURCES}
        WORKING_DIRECTORY ${ZBUILD_WORKING_DIRECTORY}
    )

    foreach(path IN ZIP_LISTS ZBUILD_OUTPUT ZBUILD_COPY_TO)
        set(src_output_file ${path_0})
        set(dest_output_dir ${path_1})

        get_filename_component(src_output_filetitle ${src_output_file} NAME)
        set(dest_output_file ${dest_output_dir}/${src_output_filetitle})

        list(APPEND DESTINATION_OUTPUT_FILES ${dest_output_file})

        add_custom_command(
            OUTPUT ${dest_output_file}
            COMMAND ${CMAKE_COMMAND} -E copy ${src_output_file} ${dest_output_file}
            DEPENDS ${src_output_file}
        )
    endforeach()

    # -------------------------------------------------------------------------------------------
    # Custom target for other targets to depend on
    # -------------------------------------------------------------------------------------------
    add_custom_target(${ZBUILD_NAME} SOURCES ${DESTINATION_OUTPUT_FILES})

    # -------------------------------------------------------------------------------------------
    # Add dependencies. So that they get build first
    # -------------------------------------------------------------------------------------------
    if (ZBUILD_DEPENDS)
        add_dependencies(${ZBUILD_NAME} ${ZBUILD_DEPENDS})
    endif()

endfunction()
