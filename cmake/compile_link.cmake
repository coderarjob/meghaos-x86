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
    add_library(${COMPILE_NAME} EXCLUDE_FROM_ALL OBJECT ${COMPILE_SOURCES})
    target_include_directories(${COMPILE_NAME} PRIVATE ${COMPILE_INCLUDE_DIRECTORIES})
    target_compile_options(${COMPILE_NAME} PRIVATE ${COMPILE_FLAGS})
endfunction()

function(link)
    set(oneValueArgs NAME)
    set(multiValueArgs DEPENDS FLAGS LINKER_FILE LINK_LIBRARIES)
    set(options FLATEN NO_LIST)
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

    add_executable(${EXE_NAME} EXCLUDE_FROM_ALL)

    if (NOT LINK_FLATEN)
        # The final binary should be in the ${MOS_BIN_DIR} directory. In case of non-flat binaries
        # the output from add_executable() is final and thus should be placed in the ${MOS_BIN_DIR}.
        set_target_properties(${EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MOS_BIN_DIR})
    endif()

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
        # The final binary should be in the ${MOS_BIN_DIR} directory. In case of flat binaries the
        # output from objdump should go to ${MOS_BIN_DIR} directory.
        set(OUT_FLAT_FILE ${MOS_BIN_DIR}/${LINK_NAME})
        add_custom_target(
            ${LINK_NAME}
            BYPRODUCTS ${OUT_FLAT_FILE}
            COMMAND ${CROSS_OBJCOPY} -O binary $<TARGET_FILE:${EXE_NAME}> ${OUT_FLAT_FILE}
            )
    endif()
    # -------------------------------------------------------------------------------------------
    # Generate listing files
    # -------------------------------------------------------------------------------------------
    if (NOT LINK_NO_LIST)
        set(OUTPUT_LIST_FILE "${MOS_LISTS_DIR}/${LINK_NAME}.lst")
        add_custom_command(
            TARGET ${EXE_NAME}
            BYPRODUCTS ${OUTPUT_LIST_FILE}
            POST_BUILD
            COMMAND ${CROSS_OBJDUMP} -dSl -M intel $<TARGET_FILE:${EXE_NAME}> > ${OUTPUT_LIST_FILE}
            COMMENT "Building listing file for ${LINK_NAME}"
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

function(test)
    set(oneValueArgs NAME DEPENDENT_FOR)
    set(multiValueArgs SOURCES INCLUDE_DIRECTORIES)
    set(options)
    cmake_parse_arguments(PARSE_ARGV 0 TEST "${options}" "${oneValueArgs}" "${multiValueArgs}")

    # -------------------------------------------------------------------------------------------
    # Check validity
    # -------------------------------------------------------------------------------------------
    if (NOT TEST_NAME)
        message(FATAL_ERROR "Name must be given.")
    endif()

    # -------------------------------------------------------------------------------------------
    # Compile and link
    # -------------------------------------------------------------------------------------------
    compile_lib(
        NAME ${TEST_NAME}.co
        SOURCES ${TEST_SOURCES}
        FLAGS ${MOS_UNITTESTS_GCC_FLAGS}
        INCLUDE_DIRECTORIES ${MOS_GCC_INCLUDE_DIRS}
        )

    link(
        NO_LIST
        NAME ${TEST_NAME}
        DEPENDS ${TEST_NAME}.co
        FLAGS ${MOS_UNITEST_LINKER_OPTIONS}
        LINK_LIBRARIES gcov m
        )

    if (TEST_DEPENDENT_FOR)
        add_dependencies(${TEST_DEPENDENT_FOR} ${TEST_NAME})
    endif()
endfunction()

