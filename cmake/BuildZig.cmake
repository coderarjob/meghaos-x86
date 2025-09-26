# ==================================================================================================
# add_zig_build (NAME name
#              ZIG_BUILD_BIN_DIR <path>
#              OUT_BIN_DIR       <path>
#              OUT_OBJ_DIR       <path>
#              OUTPUTS <output> [<outputs> ...]
#              SOURCES <source> [<source> ...]
#              [DEPENDS <target> ...]
#              [FLAGS <flag> ...]
#              [DEFINITIONS <compiler macros> ...])
#
# Invokes Zig build command build of output artifacts. These can be copied to some destination path.
#
# ZIG_BUILD_BIN_DIR
# Directory where zig builds the binaries.
# 
# OUT_BIN_DIR
# Destination directory where the FLT output file will be copied to.
#
# OUT_OBJ_DIR
# Destination directory where the ELF output file will be copied to.
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
# ==================================================================================================
function(add_zig_build)
    set(oneValueArgs NAME ZIG_BUILD_BIN_DIR OUT_BIN_DIR OUT_OBJ_DIR)
    set(multiValueArgs OUTPUTS DEPENDS SOURCES FLAGS DEFINITIONS)
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

    if (NOT ZBUILD_ZIG_BUILD_BIN_DIR)
        message(FATAL_ERROR "Output path for Zig build must be given.")
    endif()

    if (NOT ZBUILD_OUT_BIN_DIR)
        message(FATAL_ERROR "Destination path where .flt files will get copied to must be given.")
    endif()

    if (NOT ZBUILD_OUT_OBJ_DIR)
        message(FATAL_ERROR "Destination path where ELF files will get copied to must be given.")
    endif()

    list(LENGTH ZBUILD_SOURCES SOURCE_LEN)
    if (NOT SOURCE_LEN)
        message(FATAL_ERROR "There must be at least one source file.")
    endif()

    list(LENGTH ZBUILD_OUTPUTS OUTPUTS_LEN)
    if (NOT OUTPUTS_LEN)
        message(FATAL_ERROR "There must be at least one output file.")
    endif()

    # -------------------------------------------------------------------------------------------
    # Build the output list of destination files and source binary files
    # -------------------------------------------------------------------------------------------
    foreach(output IN LISTS ZBUILD_OUTPUTS)
        list(APPEND DEST_OBJS ${ZBUILD_OUT_OBJ_DIR}/${output})
        list(APPEND DEST_FLTS ${ZBUILD_OUT_BIN_DIR}/${output}.flt)
        list(APPEND SRC_OBJS ${ZBUILD_ZIG_BUILD_BIN_DIR}/${output})
        list(APPEND SRC_FLTS ${ZBUILD_ZIG_BUILD_BIN_DIR}/${output}.flt)
    endforeach()

    # -------------------------------------------------------------------------------------------
    # Command ot build the executable and copy to destination folders
    # -------------------------------------------------------------------------------------------
    add_custom_command(
        OUTPUT 
            ${DEST_OBJS}
            ${DEST_FLTS}
        BYPRODUCTS
            ${SRC_OBJS}
            ${SRC_FLTS}
        COMMAND ${ZIG_EXECUTABLE} build ${ZBUILD_FLAGS} ${ZBUILD_DEFINITIONS}
        COMMAND ${CMAKE_COMMAND} -E copy ${SRC_OBJS} ${ZBUILD_OUT_OBJ_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${SRC_FLTS} ${ZBUILD_OUT_BIN_DIR}
        DEPENDS ${ZBUILD_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    )

    # -------------------------------------------------------------------------------------------
    # Custom target for other targets to depend on
    # -------------------------------------------------------------------------------------------
    add_custom_target(
        ${ZBUILD_NAME}
        SOURCES
            ${DEST_OBJS}
            ${DEST_FLTS}
    )

    # -------------------------------------------------------------------------------------------
    # Add dependencies. So that they get build first
    # -------------------------------------------------------------------------------------------
    if (ZBUILD_DEPENDS)
        add_dependencies(${ZBUILD_NAME} ${ZBUILD_DEPENDS})
    endif()

endfunction()
