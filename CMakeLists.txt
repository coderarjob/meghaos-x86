cmake_minimum_required(VERSION 3.3)

#---------------------------------------------------------------------------
# Override few defaults for compiling with NASM
# Note: Need to be before project().
#---------------------------------------------------------------------------
set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")
# .s and .S files should be treated as assembler files.
set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS ${CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS} s S)
#---------------------------------------------------------------------------

project(MeghaOS-V2 C ASM_NASM)

set(MOS_DEBUG_LEVEL "1" CACHE STRING "Sets DEBUG_LEVEL option to this value")
set(MOS_BUILD_MODE "DEBUG" CACHE STRING "Debug mode")

set(MOS_BUILD_MODES "DEBUG" "NDEBUG")
set_property(CACHE MOS_BUILD_MODE PROPERTY STRINGS ${MOS_BUILD_MODES})
#---------------------------------------------------------------------------
# Validate User settable options
#---------------------------------------------------------------------------
if (NOT MOS_BUILD_MODE IN_LIST MOS_BUILD_MODES)
    message(FATAL_ERROR "${MOS_BUILD_MODE} is not a valid build mode.")
endif()

if (CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "MOS_BUILD_MODE should be used to setup build mode.")
endif()
#---------------------------------------------------------------------------

include(cmake/compile_assemble_link.cmake)

set(MOS_BIN_DIR                    ${CMAKE_BINARY_DIR}/bin)
set(MOS_OBJ_DIR                    ${CMAKE_BINARY_DIR}/obj)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MOS_OBJ_DIR})
set(MOS_REPOSTS_DIR                ${CMAKE_BINARY_DIR}/reports)

file(MAKE_DIRECTORY ${MOS_BIN_DIR})
file(MAKE_DIRECTORY ${MOS_REPOSTS_DIR})

set(INVOKEPROG_SCRIPT_PATH scripts/invokeprog.sh)
set(RUN_SCRIPT_PATH        scripts/run.sh)
set(POST_BUILD_SCRIPT_PATH scripts/postbuild.sh)

#---------------------------------------------------------------------------
# Just for debugging. DELETE THIS WHEN FINAL
#---------------------------------------------------------------------------
add_custom_target(debug ALL
    COMMAND ${CMAKE_COMMAND} -E echo "- CROSS COMPILING : ${CMAKE_CROSSCOMPILING}"
    COMMAND ${CMAKE_COMMAND} -E echo "- C COMPILER      : ${CMAKE_C_COMPILER}"
    COMMAND ${CMAKE_COMMAND} -E echo "- BUILD MODE      : ${MOS_BUILD_MODE}"
    COMMAND ${CMAKE_COMMAND} -E echo "- DEBUG LEVEL     : ${MOS_DEBUG_LEVEL}"
    )
#---------------------------------------------------------------------------

if(CMAKE_CROSSCOMPILING)
    include(cmake/${ARCH}/kernelflags.cmake)

    #---------------------------------------------------------------------------
    find_program(CROSS_OBJCOPY NAMES ${CMAKE_SYSTEM_PROCESSOR}-elf-objcopy REQUIRED)
    find_program(CROSS_OBJDUMP NAMES ${CMAKE_SYSTEM_PROCESSOR}-elf-objdump REQUIRED)
    #---------------------------------------------------------------------------

    set(MOS_LISTS_DIR     ${CMAKE_BINARY_DIR}/lists)
    set(MOS_DISKIMAGE_DIR ${CMAKE_BINARY_DIR}/diskimage/${ARCH})
    file(MAKE_DIRECTORY   ${MOS_LISTS_DIR})
    file(MAKE_DIRECTORY   ${MOS_DISKIMAGE_DIR})

    set(CREATE_FLOPPY_IMAGE_SCRIPT_PATH scripts/create-floppyimg.sh)
    set(LINTER_SCRIPT_PATH              scripts/lint.sh)

    #---------------------------------------------------------------------------
    add_custom_target(build-all ALL)
    add_dependencies(build-all kernel.flt boot1.flt boot0.flt proc1.flt mpdemo.flt)

    #---------------------------------------------------------------------------
    # Build & link kernel and process binaries
    #---------------------------------------------------------------------------
    add_subdirectory(src/bootloader/x86)
    add_subdirectory(src/common)
    add_subdirectory(src/kernel)
    add_subdirectory(src/kernel/${ARCH})
    add_subdirectory(src/userland)

    link(
        FLATTEN
        NAME kernel.flt
        DEPENDS 
            common
            kernel
            kernel_entry
            kernel_X86
        FLAGS ${MOS_KERNEL_LINKER_OPTIONS}
        LINKER_FILE ${MOS_KERNEL_LINKER_SCRIPT_FILE}
        LINK_LIBRARIES gcc
        )

    #---------------------------------------------------------------------------
    # Create new floppy image and the copy flat binary files
    #---------------------------------------------------------------------------
    add_custom_target(mos.flp
        DEPENDS build-all
        BYPRODUCTS ${MOS_DISKIMAGE_DIR}/mos.flp
        COMMAND ${CREATE_FLOPPY_IMAGE_SCRIPT_PATH} ${MOS_BIN_DIR} ${MOS_DISKIMAGE_DIR}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        )

    #---------------------------------------------------------------------------
    # Generate tag files post kernel.flt build
    #---------------------------------------------------------------------------
    add_custom_command(
        TARGET build-all
        POST_BUILD
        BYPRODUCTS ${PROJECT_SOURCE_DIR}/tags
                   ${PROJECT_SOURCE_DIR}/cscope.out
        COMMAND ${POST_BUILD_SCRIPT_PATH}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Tag file generation"
        )

    #---------------------------------------------------------------------------
    # Runs linter script
    #---------------------------------------------------------------------------
    set(MOS_LINT_REPORT_FILE ${MOS_REPOSTS_DIR}/lintreport.txt)
    add_custom_target(lintreport
        BYPRODUCTS ${MOS_LINT_REPORT_FILE}
        COMMAND ${LINTER_SCRIPT_PATH} ${MOS_LINT_REPORT_FILE}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        )

    #---------------------------------------------------------------------------
    # Runs the image in qemu
    # Run disk image using `make run`
    # Run disk image with arguments example: `make ARGS="-S -s" run`
    #---------------------------------------------------------------------------
    set(EMU_REPORTS_FILE ${MOS_REPOSTS_DIR}/qemulog.txt)
    add_custom_target(run
        DEPENDS mos.flp
        BYPRODUCTS ${EMU_REPORTS_FILE}
        COMMAND ${INVOKEPROG_SCRIPT_PATH} ${RUN_SCRIPT_PATH}
                --img ${MOS_DISKIMAGE_DIR}/mos.flp
                --report-out ${EMU_REPORTS_FILE}
                --  # This -- makes ARGS assignments simpler. Otherwise `--` need to be in ARGS env
                    # variable. For example `make ARGS="-- -S -s" run`.
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        )
else()
    include(cmake/${ARCH}/unittestflags.cmake)

    #---------------------------------------------------------------------------
    set(MOS_COV_DIR ${MOS_REPOSTS_DIR}/coverage)
    #---------------------------------------------------------------------------

    add_custom_target(build-all ALL)
    add_subdirectory(src/unittests)
    add_subdirectory(src/unittests/x86)

    #---------------------------------------------------------------------------
    # Generage unittest coverage report
    #---------------------------------------------------------------------------
    add_custom_target(gen-cov
        DEPENDS build-all
        BYPRODUCTS ${MOS_COV_DIR}
        # Creation of directory is required because the same is also the BYPRODUCT, which will be
        # removed after `make clean`. `file(MAKE_DIRECTORY ..)` which works at generation time so
        # will not create the directory at build time.
        COMMAND ${CMAKE_COMMAND} -E make_directory ${MOS_COV_DIR}
        COMMAND ${POST_BUILD_SCRIPT_PATH}
                --tests-dir=${MOS_BIN_DIR}
                --cov-dir=${MOS_COV_DIR}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        )
    #---------------------------------------------------------------------------
    # Runs all unittest or a particular one
    # Run every test using `make run`
    # Run single test using `make ARGS="--name <test bin name>" run`
    #---------------------------------------------------------------------------
    add_custom_target(run
        DEPENDS build-all
        BYPRODUCTS ${EMU_REPORTS_FILE}
        COMMAND ${INVOKEPROG_SCRIPT_PATH} ${RUN_SCRIPT_PATH}
                --tests-dir ${MOS_BIN_DIR}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        )
endif()
