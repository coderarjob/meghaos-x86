function(compile_lib)
    set(oneValueArgs NAME)
    set(multiValueArgs SOURCES FLAGS INCLUDE_DIRECTORIES)
    set(options)
    cmake_parse_arguments(PARSE_ARGV 0 COMPILE "${options}" "${oneValueArgs}" "${multiValueArgs}")

    # -------------------------------------------------------------------------------------------
    # Check validity
    # -------------------------------------------------------------------------------------------
    if (NOT COMPILE_NAME)
        message(FATAL_ERROR "Name must be given.")
    endif()

    list(LENGTH COMPILE_SOURCES SOURCE_LEN)
    if (NOT SOURCE_LEN)
        message(FATAL_ERROR "There must be at least one source file.")
    endif()

    # -------------------------------------------------------------------------------------------
    # Build a object library
    # -------------------------------------------------------------------------------------------
    add_library(${COMPILE_NAME} OBJECT ${COMPILE_SOURCES})
    target_include_directories(${COMPILE_NAME} PRIVATE ${COMPILE_INCLUDE_DIRECTORIES})
    target_compile_options(${COMPILE_NAME} PRIVATE ${COMPILE_FLAGS})
endfunction()

function(link)
    set(oneValueArgs NAME)
    set(multiValueArgs DEPENDS FLAGS LINKER_FILE LINK_LIBRARIES)
    set(options FLATEN)
    cmake_parse_arguments(PARSE_ARGV 0 LINK "${options}" "${oneValueArgs}" "${multiValueArgs}")

    # -------------------------------------------------------------------------------------------
    # Check validity
    # -------------------------------------------------------------------------------------------
    if (NOT LINK_NAME)
        message(FATAL_ERROR "Name must be given.")
    endif()

    list(LENGTH LINK_DEPENDS DEPENDS_LEN)
    if (NOT DEPENDS_LEN)
        message(FATAL_ERROR "There must be at least one dependency.")
    endif()

    # -------------------------------------------------------------------------------------------
    # Link to form an ELF object binary
    # -------------------------------------------------------------------------------------------
    if (LINK_FLATEN)
        # When FLATTEN option is set, then the linked executabled is an intermediate the flattened
        # binary is the final. That is the reason for the '.in' suffix - standing for intermediate.
        set(EXE_NAME ${LINK_NAME}.${MOS_INTERMEDIATE_EXTENSION})
    else()
        set(EXE_NAME ${LINK_NAME})
    endif()

    add_executable(${EXE_NAME})

    foreach(dependency IN LISTS LINK_DEPENDS)
        target_sources(${EXE_NAME} PRIVATE $<TARGET_OBJECTS:${dependency}>)
    endforeach()

    target_link_options(${EXE_NAME} PRIVATE ${LINK_FLAGS})
    if (LINK_LINKER_FILE)
        target_link_options(${EXE_NAME} PRIVATE -T ${LINK_LINKER_FILE})
        set_target_properties(${EXE_NAME} PROPERTIES LINK_DEPENDS ${LINK_LINKER_FILE})
    endif()

    if (LINK_LINK_LIBRARIES)
        target_link_libraries(${EXE_NAME} PRIVATE ${LINK_LINK_LIBRARIES})
    endif()
    # -------------------------------------------------------------------------------------------
    # Flaten ELF binary is 'FLATEN' option is ON
    # -------------------------------------------------------------------------------------------
    # Note: Due to use of TARGET_FILE gen expression, the dependency between <LINK_NAME> and 
    # <EXE_NAME> targets is added automatically.
    if (LINK_FLATEN)
        set(OUT_FLAT_FILE ${MOS_BIN_DIR}/${LINK_NAME})
        add_custom_target(
            ${LINK_NAME}
            BYPRODUCTS ${OUT_FLAT_FILE}
            COMMAND ${CROSS_OBJDIR} -O binary $<TARGET_FILE:${EXE_NAME}> ${OUT_FLAT_FILE}
            )
    endif()
endfunction()
    
function(copy_object_file)
    set(multiValueArgs)
    set(options)
    set(oneValueArgs NAME DEPENDS OUTPUT_DIRECTORY)
    cmake_parse_arguments(PARSE_ARGV 0 CPOBJ "${options}" "${oneValueArgs}" "${multiValueArgs}")

    # -------------------------------------------------------------------------------------------
    # Check validity
    # -------------------------------------------------------------------------------------------
    if (NOT CPOBJ_NAME OR NOT CPOBJ_DEPENDS OR NOT CPOBJ_OUTPUT_DIRECTORY)
        message(FATAL_ERROR "Name and dependency and output directory must be given.")
    endif()

    # -------------------------------------------------------------------------------------------
    # Copy the object file produced by the 'Dependency' to the 'output directory'
    # -------------------------------------------------------------------------------------------
    set(OUT_FLAT_FILE ${CPOBJ_OUTPUT_DIRECTORY}/${CPOBJ_NAME})
    add_custom_target(
        ${CPOBJ_NAME}
        BYPRODUCTS ${OUT_FLAT_FILE}
        DEPENDS ${CPOBJ_DEPENDS}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:${CPOBJ_DEPENDS}> ${OUT_FLAT_FILE}
        )
endfunction()

