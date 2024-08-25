# ==================================================================================================
# compile_lib (NAME name
#              SOURCES <source> [<source> ...]
#              [FLAGS <flag> ...]
#              [INCLUDE_DIRECTORIES <paths> ...])
#
# Creates object files for the sources provided. The object files can be accessed later using the
# generator expression $<TARGET_OBJECTS:name>.
# ==================================================================================================
function(compile_lib)
    set(oneValueArgs NAME)
    set(multiValueArgs SOURCES FLAGS INCLUDE_DIRECTORIES)
    set(options)
    cmake_parse_arguments(PARSE_ARGV 0 COMPILE "${options}" "${oneValueArgs}" "${multiValueArgs}")

    # -------------------------------------------------------------------------------------------
    # Check validity
    # -------------------------------------------------------------------------------------------
    if (COMPILE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Invalid option: ${COMPILE_UNPARSED_ARGUMENTS}")
    endif()

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

# ==================================================================================================
# link (NAME name
#       DEPENDS <target> [<target> ...]
#       [RESOURCES <path to resources to add>]
#       [FLAGS <linker flags>]
#       [LINKER_FILE <path to link script>]
#       [LINK_LIBRARIES <libraries to link>]
#       [FLATTEN]
#       [NO_LIST])
#
# Links object files associated with the dependencies and produces a single binary (flattened or
# not) in MOS_BIN_DIR directory. Also produces listing file in MOS_LISTS_DIR unless NO_LIST option
# is given.
#
# NAME
# Target name associated with the final binary produced. If FLATTEN option is given then  produces
# two targets:
# 1. <name>.in - For the `add_executable` which produces the linked ELF binary. However as it is
#                not the final file the target of this is appended with `.in`.
# 2. <name>    - For the custom target producing the final flat binary.
#
# If FLATTEN option is not given then there is only one target:
# 1. <name>    - For the `add_executable` which produces the linked ELF executable.
#
# DEPENDS
# Names from `compile_lib` which this depends on.
#
# RESOURCES
# These files will be copied to the MOS_BIN_DIR directory.
#
# FLAGS
# Linker flags
#
# LINK_LIBRARIES
# List of external libraries which this depends on.
#
# LINKER_FILE
# Path to linker file. If provided `-T` option will be added to the FLAGS and the file will be added
# as dependency for the `add_executable` target.
#
# FLATTEN
# Produces flat binaries in MOS_BIN_DIR, using the linked ELF binary.
#
# NO_LIST
# Does not produce listing files.
# ==================================================================================================
function(link)
    set(oneValueArgs NAME)
    set(multiValueArgs DEPENDS RESOURCES FLAGS LINKER_FILE LINK_LIBRARIES)
    set(options FLATTEN NO_LIST)
    cmake_parse_arguments(PARSE_ARGV 0 LINK "${options}" "${oneValueArgs}" "${multiValueArgs}")

    # -------------------------------------------------------------------------------------------
    # Check validity
    # -------------------------------------------------------------------------------------------
    if (LINK_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Invalid option: ${LINK_UNPARSED_ARGUMENTS}")
    endif()

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
    if (LINK_FLATTEN)
        # When FLATTEN option is set, then the linked executabled is an intermediate the flattened
        # binary is the final. That is the reason for the '.in' suffix - standing for intermediate.
        set(INTERMEDIATE_EXTENSION "in")
        set(EXE_NAME ${LINK_NAME}.${INTERMEDIATE_EXTENSION})
    else()
        set(EXE_NAME ${LINK_NAME})
    endif()

    add_executable(${EXE_NAME} EXCLUDE_FROM_ALL)

    if (NOT LINK_FLATTEN)
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
    # Flatten ELF binary is 'FLATTEN' option is ON
    # -------------------------------------------------------------------------------------------
    # Note: Due to use of TARGET_FILE gen expression, the dependency between <LINK_NAME> and 
    # <EXE_NAME> targets is added automatically.
    if (LINK_FLATTEN)
        # The final binary should be in the ${MOS_BIN_DIR} directory. In case of flat binaries the
        # output from objdump should go to ${MOS_BIN_DIR} directory.
        set(OUT_FLAT_FILE ${MOS_BIN_DIR}/${LINK_NAME})
        add_custom_target(${LINK_NAME} DEPENDS ${OUT_FLAT_FILE})
        add_custom_command(
            OUTPUT ${OUT_FLAT_FILE}
            DEPENDS ${EXE_NAME}
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
    # -------------------------------------------------------------------------------------------
    # Copy files in RESOURCES list to MOS_BIN_DIR
    # -------------------------------------------------------------------------------------------
    foreach(resource IN LISTS LINK_RESOURCES)
        get_filename_component(FILE_TITLE ${resource} NAME)
        set(OUTPUT_RES_FILE "${MOS_BIN_DIR}/${FILE_TITLE}")
        add_custom_target(${FILE_TITLE} DEPENDS ${OUTPUT_RES_FILE})
        add_custom_command(
            OUTPUT ${OUTPUT_RES_FILE}
            DEPENDS ${resource}
            COMMAND ${CMAKE_COMMAND} -E copy ${resource} ${OUTPUT_RES_FILE}
        )
        add_dependencies(${EXE_NAME} ${FILE_TITLE})
    endforeach()

endfunction()
    
# ==================================================================================================
# copy_object_file (NAME name
#                   DEPENDS <target>
#                   OUTPUT_DIRECTORY <path where to copy>)
#
# Copies object files associated with `compile_lib` dependency to OUTPUT_DIRECTORY directory.
# ==================================================================================================
function(copy_object_file)
    set(multiValueArgs)
    set(options)
    set(oneValueArgs NAME DEPENDS OUTPUT_DIRECTORY)
    cmake_parse_arguments(PARSE_ARGV 0 CPOBJ "${options}" "${oneValueArgs}" "${multiValueArgs}")

    # -------------------------------------------------------------------------------------------
    # Check validity
    # -------------------------------------------------------------------------------------------
    if (CPOBJ_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Invalid option: ${CPOBJ_UNPARSED_ARGUMENTS}")
    endif()

    if (NOT CPOBJ_NAME OR NOT CPOBJ_DEPENDS OR NOT CPOBJ_OUTPUT_DIRECTORY)
        message(FATAL_ERROR "Name and dependency and output directory must be given.")
    endif()

    # -------------------------------------------------------------------------------------------
    # Copy the object file produced by the 'Dependency' to the 'output directory'
    # -------------------------------------------------------------------------------------------
    set(OUT_FLAT_FILE ${CPOBJ_OUTPUT_DIRECTORY}/${CPOBJ_NAME})
    add_custom_target(${CPOBJ_NAME} DEPENDS ${OUT_FLAT_FILE})
    # NOTE: DEPENDS includes both the target name (${CPOBJ_NAME} as well as the corresponding object
    # file ($<TARGET_OBJECTS:${CPOBJ_DEPENDS}>). I am not sure why both is required, but I think its
    # because the target is a OBJECT library type, otherwise it would not have been required.
    add_custom_command(
        OUTPUT ${OUT_FLAT_FILE}
        DEPENDS ${CPOBJ_DEPENDS} $<TARGET_OBJECTS:${CPOBJ_DEPENDS}>
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:${CPOBJ_DEPENDS}> ${OUT_FLAT_FILE}
    )
endfunction()

# ==================================================================================================
# test (NAME name
#       [DEPENDENT_FOR target]
#       [SOURCES <source> [<source> ...]
#       [INCLUDE_DIRECTORIES <paths> ..])
#
# Produces test executable from the source files. Binary files so produced are placed in
# MOS_BIN_DIR. It is added as a dependency for DEPENDENT_FOR.
# ==================================================================================================
function(test)
    set(oneValueArgs NAME DEPENDENT_FOR)
    set(multiValueArgs SOURCES INCLUDE_DIRECTORIES)
    set(options)
    cmake_parse_arguments(PARSE_ARGV 0 TEST "${options}" "${oneValueArgs}" "${multiValueArgs}")

    # -------------------------------------------------------------------------------------------
    # Check validity
    # -------------------------------------------------------------------------------------------
    if (TEST_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Invalid option: ${TEST_UNPARSED_ARGUMENTS}")
    endif()

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

# ==================================================================================================
# assemble_and_copy_bin (NAME name
#                        SOURCES <source> [<source> ...]
#                        [FLAGS <assembler flag> ...]
#                        [INCLUDE_DIRECTORIES <paths> ..])
#
# Compiles a assembly files using NASM then copies each object files to MOS_BIN_DIR.
# ==================================================================================================
function(assemble_and_copy_bin)
    set(oneValueArgs NAME)
    set(multiValueArgs SOURCES FLAGS INCLUDE_DIRECTORIES)
    set(options)
    cmake_parse_arguments(PARSE_ARGV 0 ASSEMBLE "${options}" "${oneValueArgs}" "${multiValueArgs}")

    # -------------------------------------------------------------------------------------------
    # Check validity
    # -------------------------------------------------------------------------------------------
    if (ASSEMBLE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Invalid option: ${ASSEMBLE_UNPARSED_ARGUMENTS}")
    endif()

    # -------------------------------------------------------------------------------------------
    # Compile and copy binary files to destinations
    # -------------------------------------------------------------------------------------------
    set(INTERMEDIATE_BIN_NAME ${ASSEMBLE_NAME}-lib)
    compile_lib(
        NAME ${INTERMEDIATE_BIN_NAME}
        SOURCES ${ASSEMBLE_SOURCES}
        FLAGS ${ASSEMBLE_FLAGS}
        INCLUDE_DIRECTORIES ${ASSEMBLE_INCLUDE_DIRECTORIES}
        )
    copy_object_file(
        NAME ${ASSEMBLE_NAME}
        DEPENDS ${INTERMEDIATE_BIN_NAME}
        OUTPUT_DIRECTORY ${MOS_BIN_DIR}
        )
endfunction()
