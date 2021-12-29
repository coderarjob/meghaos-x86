#!/bin/bash

pushd ~/meghaos-v2/

# --------------------------------------------------------------------------
# Integer types
# --------------------------------------------------------------------------
P=u8
F=U8
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;
P=u16
F=U16
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;
P=u32
F=U32
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=u64
F=U64
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=unsigned\ long\ long\ int
F=ULLONG
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=long\ long\ int
F=LLONG
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=unsigned\ long\ int
F=ULONG
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=long\ int
F=LONG
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=unsigned\ int
F=UINT
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=int
F=INT
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=unsigned\ char
F=UCHAR
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=char\ *
F=CHAR
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=char
F=CHAR
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

     # ---- Appy Fix for (char -> CHAR)
     FILE=include/x86/types.h
     sed -i "s/\<typedef CHAR\>/typedef char/g" $FILE
     # --------------------------------

     # ---- Appy Fix for (unsigned char -> UCHAR)
     FILE=include/x86/types.h
     sed -i "s/\<typedef UCHAR\>/typedef unsigned char/g" $FILE
     # --------------------------------

# --------------------------------------------------------------------------
# Macro names
# --------------------------------------------------------------------------
P=kstatic_assert
F=k_staticAssert
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=kassert
F=k_assert
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=kassert_perror
F=k_assertOnError
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=disp_attr
F=k_dispAttr
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=khalt
F=k_halt
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=OFFSET_OF
F=offsetOf
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=printk_debug
F=kdebug_printf
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

# --------------------------------------------------------------------------
# Function names
# --------------------------------------------------------------------------
P=kdisp_scrolldown
F=kdisp_scrollDown
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=printk
F=kearly_printf
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=kpanic
F=k_panic
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=vsnprintk
F=kearly_vsnprintf
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=printk_debug_gs
F=kdebug_printf_ndu
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=memcpy
F=k_memcpy
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=memset
F=k_memset
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=kpanic_gs
F=k_panic_ndu
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

# --------------------------------------------------------------------------
# Struct
# --------------------------------------------------------------------------
P=struct\ mem_des
F=BootMemoryMapItem
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=struct\ file_des
F=BootFileItem
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=struct\ boot_info
F=BootLoaderInfo
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=struct\ pte
F=PageTableEntry
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=struct\ pde4mb
F=PageDirectoryEntry4MB
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=struct\ pde4kb
F=PageDirectoryEntry4KB
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;
# --------------------------------------------------------------------------
# Enums
# --------------------------------------------------------------------------
P=enum\ disp_ioctl
F=DisplayControls
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=enum\ vga_colors
F=DisplayVgaColors
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=enum\ idt_des_types
F=IDTDescriptorTypes
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;
# --------------------------------------------------------------------------
# Global Variables
# --------------------------------------------------------------------------
P=kerrno
F=k_errorNumber
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;

P=kerrdes
F=k_errorText
find -type f \( -name "*.c" -o -name "*.h" \) \
     -exec sed -i "s/\<$P\>/$F/g" {} \;
# --------------------------------------------------------------------------
popd
