
; Lists down all the error codes that can occour in the kernel or any of the
; system modules that make up the OS.

K_ERR_NONE:	equ 0

; General/Common Errors
K_ERR_FILE_NOT_FOUND:			EQU 10

; Errors in mem.s
K_ERR_MEM_STORAGE_FULL:			EQU 20

; Errors in Process (Kernel)
K_ERR_PROCESS_PARENT_ZERO:		EQU 40
K_ERR_PROCESS_LIST_FULL:		EQU 41
K_ERR_PROCESS_NOT_FOUND:		EQU 42

; Errors in various kernel modules. (May not be available outside the kernel
; module)
ERR_NOTI_FULL:					EQU 1
ERR_ALLOC_STORAGE_FULL:			EQU 0
ERR_FREE_SEGMENT_NOT_FOUND:		EQU 1
ERR_INVALID_PROCESS_ID:			EQU 1
