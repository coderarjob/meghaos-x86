; Main include file for kernel, driver and application developers.
; It includes the function numbers for all interrupts in the MOS kernel.
; It also includes some version information of various files in this release of
; the MOS Operating System.

%ifndef __MOS_INC__
	%define __MOS_INC__

    ; Helper macros
    ; ------------------------------------------------------------------------
    %define HIGH_BITS(a,b) (a) >> (b)
    %define LOW_BITS(a,b)  (a) & ((1 << (b)) -1)

    ; Creates constants for segment and offset values from the given address.
    ; %1    - Label         (name for the memory location)
    ; %2    - 32 bit address (name for the memory location)
    %macro MEM_LOC 2
        %1_MEM      EQU     %2
        %1_SEG      EQU     HIGH_BITS(%2, 16)
        %1_OFF      EQU     LOW_BITS(%2, 16)
    %endmacro

    ; Macros to calculate Physical from higher half mapped Virutal addresses
    ; and vice-versa.
    %define PHY(m) (m - 0xC0000000)
    %define HIGHV(m) (m + 0xC0000000)

	; Version informations
    ; ------------------------------------------------------------------------
	%define     BOOT0_BUILD 	"20211224"
	%define     BOOT1_BUILD 	"20220617"
	%define 	OS_NAME 	    "MEGHA OPERATING SYSTEM V2 - x86"

	; MOS Fixed Memory Locations
    ; ------------------------------------------------------------------------
	MEM_LOC     GDT,              0x0000_0800  ; 256 interupts *8 = 0x800
	MEM_LOC     BOOT_INFO,        0x0000_1800
	MEM_LOC     LOADER,           0x0000_8000  ; 7C00h + 512 + 512 = 8000h
	MEM_LOC     FILE_BUFF,        0x2800_0000  ; 0000:8000h + 128KB = 28000
	MEM_LOC     KERNEL_STACK_TOP, 0x0002_7FFF  ; Stack is 128 KB in size
	MEM_LOC     KERNEL_IMG,       0x0010_0000  ; Kernel image is copied here.

    ; Constants
    ; ------------------------------------------------------------------------
    MAX_FILES_COUNT:    EQU     11              ; 10 Ramdisks + 1 Kernel file.
    MIN_MEM_REQ:        EQU     4 * 1024 * 1024 ; 4 MB RAM required mininum.

    %define BOOT1_FILE      "BOOT1   FLT"
    %define KERNEL_FILE     "KERNEL  FLT"
    %define RAMDISK0_FILE   "RAMDISK0FLT"

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
