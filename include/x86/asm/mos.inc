; Main include file for kernel, driver and application developers.
; It includes the function numbers for all interrupts in the MOS kernel.
; It also includes some version information of various files in this release of
; the MOS Operating System.

%ifndef __MOS_INC__
	%define __MOS_INC__

    ; Helper macros
    ; ------------------------------------------------------------------------
    %define HIGH_BITS(a,b) ((a) >> (b))
    %define LOW_BITS(a,b)  ((a) & ((1 << (b)) -1))

    ; Creates constants for segment and offset values from the given address.
    ; %1    - Label          (name for the memory location)
    ; %2    - 32 bit address (16 bits segment _ 16 bits offset)
    %macro SEG_MEM_LOC 2
        %1_SEG      EQU     HIGH_BITS(%2,16)
        %1_OFF      EQU     LOW_BITS(%2,16)
        %1_MEM      EQU     (%1_SEG << 4) + %1_OFF
    %endmacro

    ; Creates memory constant for non segmented memory address.
    ; %1    - Label          (name for the memory location)
    ; %2    - 32 bit address
    %macro MEM_LOC 2
        %1_MEM      EQU     %2
    %endmacro

    ; Macros to calculate Physical from higher half mapped Virutal addresses
    ; and vice-versa.
    %define PHY(m) (m - 0xC0000000)
    %define HIGHV(m) (m + 0xC0000000)

	; Version informations
    ; ------------------------------------------------------------------------
	%define     BOOT0_BUILD 	"20211224"
	%define     BOOT1_BUILD 	"20220710"
	%define 	OS_NAME 	    "MEGHA OPERATING SYSTEM V2 - x86"

	; MOS Fixed Memory Locations
    ; ------------------------------------------------------------------------
    ; Segmented Memories          SEG   : OFF
	SEG_MEM_LOC     GDT,          0x0000_1000
	SEG_MEM_LOC     BOOT_INFO,    0x0000_2000
	SEG_MEM_LOC     LOADER,       0x0000_8000  ; 7C00h + 512 + 512 = 8000h
	SEG_MEM_LOC     FILE_BUFF,    0x1800_0000  ; 0000:8000h + 64KB = 18000

    ; Non segmented Memories
	MEM_LOC     KERNEL_STACK_TOP, 0x0002_2FFF
	MEM_LOC     KERNEL_IMG,       0x0010_0000  ; Kernel image is copied here.
	MEM_LOC     KERNEL_PAGE_DIR,  0x0002_3000
	MEM_LOC     KERNEL_PAGE_TABLE,0x0002_4000
	MEM_LOC     KERNEL_PAB,       0x0002_5000

    ; Constants
    ; ------------------------------------------------------------------------
    MAX_FILES_COUNT:    EQU     11              ; 10 Ramdisks + 1 Kernel file.
    MIN_MEM_REQ:        EQU     2 * 1024 * 1024 ; 2 MB RAM required mininum.

    %define BOOT1_FILE      "BOOT1   FLT"
    %define KERNEL_FILE     "KERNEL  FLT"
    %define PROC1_FILE      "PROC1   FLT"
    %define MPDEMO_FILE     "MPDEMO  FLT"

	; Structures
    ; ------------------------------------------------------------------------
	struc mem_des_t
        .BaseAddrLow   : resd 1
        .BaseAddrHigh  : resd 1
        .LengthLow     : resd 1
        .LengthHigh    : resd 1
        .Type          : resd 1
	endstruc

	struc file_des_t
        .StartLocation : resd 1
        .Length        : resw 1
	endstruc

	struc boot_info_t
        .file_count    : resw 1
        .file_dec_items: resb file_des_t_size * MAX_FILES_COUNT
        .mem_des_count : resw 1
        .mem_des_items : resb mem_des_t_size
	endstruc
%endif
