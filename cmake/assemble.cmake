find_program(NASM NAMES nasm)
if (NOT NASM)
    message(FATAL_ERROR "nasm assembler required and was not found.")
endif()

function(assemble)
    set(oneValueArgs TARGET SOURCE OUTFILE LISTDIR)
    set(multiValueArgs FLAGS INCLUDE_DIRECTORIES)
    set(options)
    cmake_parse_arguments(PARSE_ARGV 0 ASSEMBLE "${options}" "${oneValueArgs}" "${multiValueArgs}")

    # -------------------------------------------------------------------------------------------
    # Check validity
    # -------------------------------------------------------------------------------------------
    if (NOT ASSEMBLE_TARGET OR NOT ASSEMBLE_SOURCE OR NOT ASSEMBLE_OUTFILE)
        message(FATAL_ERROR "Target, assembly source and output binary filename must be given.")
    endif()

    # -------------------------------------------------------------------------------------------
    # Directories
    # -------------------------------------------------------------------------------------------
    set(ASM_BIN_FILE ${ASSEMBLE_OUTFILE})
    if (ASSEMBLE_LISTDIR)
        get_filename_component(list_file_title ${ASSEMBLE_SOURCE} NAME_WE)
        set(ASM_LIST_FILE "${ASSEMBLE_LISTDIR}/${list_file_title}.lst")
    endif()

    # -------------------------------------------------------------------------------------------
    # Gather include directories
    # -------------------------------------------------------------------------------------------
    foreach(DIR ${ASSEMBLE_INCLUDE_DIRECTORIES})
        set(ASM_INC_DIR_FLAGS ${ASM_INC_DIR_FLAGS} -I ${DIR})
    endforeach()

    # -------------------------------------------------------------------------------------------
    # Invoke nasm to build bin and list files
    # -------------------------------------------------------------------------------------------
    add_custom_command(
        OUTPUT ${ASM_BIN_FILE} ${ASM_LIST_FILE}
        DEPENDS ${ASSEMBLE_SOURCE}
        COMMAND ${NASM} ${ASSEMBLE_FLAGS} ${ASSEMBLE_SOURCE}
                ${ASM_INC_DIR_FLAGS}
                -o ${ASM_BIN_FILE} -l ${ASM_LIST_FILE}
        COMMENT "Building ${ASSEMBLE_TARGET}"
        )

    add_custom_target(${ASSEMBLE_TARGET} ALL
        DEPENDS ${ASM_BIN_FILE} ${ASM_LIST_FILE}
        )
endfunction()
