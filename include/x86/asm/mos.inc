; Main include file for kernel, driver and application developers.
; It includes the function numbers for all interrupts in the MOS kernel.
; It also includes some version information of various files in this release of
; the MOS Operating System.

%ifndef __MOS_INC__
	%define __MOS_INC__

	; Version informations
	%define     BOOT0_BUILD 	"20211224"
	%define     BOOT1_BUILD 	"20220110"
	%define 	OS_NAME 	    "MEGHA OPERATING SYSTEM V2 - x86"

	; MOS Fixed Memory Locations
	LOADER_SEG:		EQU		0x0000
	LOADER_OFF:		EQU		0x8000          ; 7C00h + 512 + 512 = 8000h

	FILE_BUFF_SEG:	EQU		0x2800          ; 0000:8000h + 128KB = 28000
	FILE_BUFF_OFF:	EQU		0x0000          ; 28000 = 0x2800:0x0000

	GDT_SEG:		EQU		0x0000
	GDT_OFF:		EQU		0x0800          ; 256 interupts *8 = 0x800

    KSTACK_TOP:     EQU     0x27FFF         ; Stack is 128 KB in size

    BOOT_INFO_SEG:  EQU     0x0000          ; Location of the multiboot_info
    BOOT_INFO_OFF:  EQU     0x1800          ; structure, as filled by 'boot1'

    MEM_AVL_MIN:    EQU     4 * 1024 * 1024 ; 4 MB RAM required mininum.

    KERNEL_IMG_MEM: EQU     0x100000        ; Kernel image is copied here.

    ; Constants
    MAX_FILES_COUNT EQU     11              ; 10 Ramdisks + 1 Kernel file.

    ; Filenames
    %define BOOT1_FILE      "BOOT1   FLT"
    %define KERNEL_FILE     "KERNEL  FLT"
    %define RAMDISK0_FILE   "RAMDISK0FLT"
%endif
