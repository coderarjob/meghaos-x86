set(BOOTLOADER_0_SOURCES
    boot0/boot.s
    boot0/loadFile.s
    )

add_compile_options(-f bin)
add_executable(boot0 ${BOOTLOADER_0_SOURCES})
