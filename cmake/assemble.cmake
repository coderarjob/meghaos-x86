function(assemble)
    set(oneValueArgs TARGET SOURCE COPYTO LISTDIR)
    set(multiValueArgs FLAGS INCLUDE_DIRECTORIES)
    set(options)
    cmake_parse_arguments(PARSE_ARGV 0 ASSEMBLE "${options}" "${oneValueArgs}" "${multiValueArgs}")

    # -------------------------------------------------------------------------------------------
    # Check validity
    # -------------------------------------------------------------------------------------------
    if (NOT ASSEMBLE_TARGET OR NOT ASSEMBLE_SOURCE OR NOT ASSEMBLE_FLAGS)
        message(FATAL_ERROR "Target, Assembly source and flags must be given.")
    endif()

    if (NOT ASSEMBLE_COPYTO)
        message(FATAL_ERROR "COPYTO must be provided.")
    endif()

    # -------------------------------------------------------------------------------------------
    # Object target requirements
    # -------------------------------------------------------------------------------------------
    add_library(${ASSEMBLE_TARGET} OBJECT ${ASSEMBLE_SOURCE})
    target_include_directories(${ASSEMBLE_TARGET} PUBLIC ${ASSEMBLE_INCLUDE_DIRECTORIES})

    # -------------------------------------------------------------------------------------------
    # Add assembler config to generate list files and cmake rule to copy the list file to
    # the provided directory.
    # -------------------------------------------------------------------------------------------
    if (ASSEMBLE_LISTDIR)
        set(list_file "${ASSEMBLE_LISTDIR}/${ASSEMBLE_TARGET}.lst")
        list(APPEND ASSEMBLE_FLAGS -l ${list_file})

        add_custom_command(
            OUTPUT ${list_file}
            DEPENDS $<TARGET_OBJECTS:${ASSEMBLE_TARGET}>
            COMMAND ${CMAKE_COMMAND} -E touch ${list_file}
            )

    endif()

    target_compile_options(${ASSEMBLE_TARGET} PRIVATE ${ASSEMBLE_FLAGS})

    # -------------------------------------------------------------------------------------------
    # Copy to a destination directory
    # -------------------------------------------------------------------------------------------
    add_custom_command(
        OUTPUT ${ASSEMBLE_COPYTO}
        DEPENDS $<TARGET_OBJECTS:${ASSEMBLE_TARGET}>
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:${ASSEMBLE_TARGET}> ${ASSEMBLE_COPYTO}
        )

endfunction()
