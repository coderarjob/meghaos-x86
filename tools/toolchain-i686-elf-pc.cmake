set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR i686)

set(CROSSCOMPILE_PATH ~/.local/opt/i686-cross)
set(ARCH x86)

set(MOS_ASM_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include/asm
    ${PROJECT_SOURCE_DIR}/include/x86/asm
    )

set(MOS_GCC_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include
    )

set(MOS_GCC_WARN_FLAGS
    -Wpedantic
    -Wall
    -Wextra
    -Wconversion
    -Wdangling-else
    -Werror
    )

set(MOS_GCC_FLAGS
    ${MOS_GCC_WARN_FLAGS}
    -std=c99
    -g
    -nostartfiles
    -ffreestanding
    -fno-pie
    -fno-stack-protector
    -fno-asynchronous-unwind-tables
    -m32
    -march=i386
    -masm=intel
    -mno-red-zone
    -mno-sse
    -O1
    -fno-unit-at-a-time
    -fno-omit-frame-pointer
    -fno-inline-functions-called-once
    -DDEBUG
    -DDEBUG_LEVEL=1
    )

set(MOS_LINKER_FLAGS_KERNEL
    -T ${PROJECT_SOURCE_DIR}/build/kernel.ld
    )

set(MOS_LINKER_OPTIONS
    -ffreestanding
    -nostdlib
    )

set(CMAKE_C_COMPILER i686-elf-gcc)
set(CMAKE_FIND_ROOT_PATH ${CROSSCOMPILE_PATH})
set(CMAKE_SYSROOT ${CROSSCOMPILE_PATH})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")
set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS ${CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS} s S)

find_program(CROSS_OBJDIR NAMES i686-elf-objcopy PATHS ${CROSSCOMPILE_PATH})
