; Main include file for kernel, driver and application developers.
; It includes the function numbers for all interrupts in the MOS kernel.
; It also includes some version information of various files in this release of
; the MOS Operating System.

%ifndef __MOS_INC__
	%define __MOS_INC__

	; Version informations
	%define     BOOT0_BUILD 	"20200919"
	%define     BOOT1_BUILD 	"20200909"
	%define 	OS_NAME 	    "MEGHA OPERATING SYSTEM V2 - x86"

	; Error Codes in the OS Kernel and Drivers/Modules.
	%include "errno.inc"

	; MOS Fixed Memory Locations
	LOADER_SEG		EQU		0x0000
	LOADER_OFF:		EQU		0x8000          ; 7C00h + 512 + 512 = 8000h
	KERNEL_SEG:		EQU		0x1800          ; 0000:8000h + 65KB = 18000 
	KERNEL_OFF:		EQU		0x0000          ; 18000 = 0x1800:0x0000
	GDT_SEG:		EQU		0x0000
	GDT_OFF:		EQU		0x0800          ; 256 interupts *8 = 0x800

    ; Filenames
    %define BOOT1_FILE      "BOOT1   FLT"
    %define KERNEL_FILE     "KERNEL  FLT"
%endif
