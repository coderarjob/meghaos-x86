function(assemble_and_copy_bin)
    set(oneValueArgs NAME)
    set(multiValueArgs SOURCES FLAGS INCLUDE_DIRECTORIES)
    set(options)
    cmake_parse_arguments(PARSE_ARGV 0 ASSEMBLE "${options}" "${oneValueArgs}" "${multiValueArgs}")

    # -------------------------------------------------------------------------------------------
    # Compile and copy binary files to destinations
    # -------------------------------------------------------------------------------------------
    set(INTERMEDIATE_BIN_NANME ${ASSEMBLE_NAME}.${MOS_INTERMEDIATE_EXTENSION})
    compile_lib(
        NAME ${INTERMEDIATE_BIN_NANME}
        SOURCES ${ASSEMBLE_SOURCES}
        FLAGS ${ASSEMBLE_FLAGS}
        INCLUDE_DIRECTORIES ${ASSEMBLE_INCLUDE_DIRECTORIES}
        )
    copy_object_file(
        NAME ${ASSEMBLE_NAME}
        DEPENDS ${INTERMEDIATE_BIN_NANME}
        OUTPUT_DIRECTORY ${MOS_BIN_DIR}
        )
endfunction()
