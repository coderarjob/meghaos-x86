# Megha Operating System V2
------------------------------------------------------------------------------

##  Equivalence of ELF and flat binary files before and after CMake
categories: note, independent
25 May 2024

Binary equivalence of ELF object files depend on two factors:
    1. Build mode is NDEBUG
    2. `-g` compilation option is not added.

If they are not met then the ELF object files will differ in size and context. This difference comes
due to the source file paths stored in debugging sections and in `.rodata` section for k_assert. For
flat binaries there are no debugging data, but the data in `.rodata` section differences still
remains.

If the above two conditions are met, then the ELF object files are binary equivalent i.e kmalloc.o
(from build.sh) and kmalloc.c.obj (from cmake) are equivalent.

When the above conditions are met the flat binaries (can be produced using `objdump -O binary <obj
file path>`) of these object files are also binary equivalent.

When these ELF object files are linked to produce Kernel ELF binary the binaries may differ in size
and content. This difference is due to the order in which the object files are linked and padding
bytes. These differences translate to the flat binaries produced from it.

However for single source file programs (in case of mpdemo & prog1) there can be no ordering issues
and thus we see no there is no difference either in the ELF files produced after linking or the flat
binaries produced for it.

Here are some data:
```
---------------|--------|----------|-------|------------|--------------------------------------------
                cmake     build.sh
                sizes     sizes      Mode    Difference   Reason
---------------|--------|----------|-------|------------|--------------------------------------------
Kernel.elf        39496    39448     NDEBUG   048 bytes   Extra padding bytes due to linking order.
kernel.flt        25128    25104     NDEBUG   024 bytes   Extra padding bytes due to linking order.
                  35936    35672     DEBUG    264 bytes   DEBUG mode has source file info.
__FILE__s         35472    35464     DEBUG    008 bytes   Extra padding bytes due to linking order.
removes
---------------|--------|----------|-------|------------|--------------------------------------------
mpdemo.elf        5536     5536      NDEBUG   000 bytes   Equivallent
mpdemo.flt        872      872       NDEBUG   000 bytes   Equivallent
---------------|--------|----------|-------|------------|--------------------------------------------
```

