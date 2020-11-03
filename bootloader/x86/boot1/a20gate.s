;
; Megha Operating System V2 - x86 A20 Routines
; ---------------------------------------------------------------------------
;  This file houses routines that deals with detection and enabling of the A20
;  Gate in x86 architecture.
;
;  Note: The loader is a transient program, so no Interrupt routine, System
;  calls must be placed in the loader.
;
;  Dated: 3rd September 2020
; ---------------------------------------------------------------------------
;  Change Log
; ---------------------------------------------------------------------------
;  Version 20200903
;  - Initial version
; ---------------------------------------------------------------------------

; Enables A20 line using FAST A20 method
; Input:    None
; Output:   None
__enable_a20_fast:
	push eax
		in al, 0x92
		or al, 2
		out 0x92, al
	pop eax
    ret

; Enables A20 line using BIOS INT 15 routine
; Input:    None
; Output:   None
__enable_a20gate_bios:
	push eax
		mov eax, 0x2401
		int 0x15
	pop eax
    ret

; If the PIN 20 (Bit 21) is disabled, then address from 
; 1 0000 0000 0000 0000 - 1 1111 1111 1111 1111, has the same meaning as
; 0 0000 0000 0000 0000 - 0 1111 1111 1111 1111. 
; Address 0x500 is chosen, because that address is not used by BIOS 
; and not part of BDA.
; We will write byte 0x00 at location 0 0000 0000 0101 0000 0000
; and 0x01 at location                1 0000 0000 0101 0000 0000. 
; When read back, if both the bytes are the same, then A20 line is disabled, 
; else enabled.
; Input:
;   None
; Output:
;   Carry  - 1 (enabled), 0 (disabled)
__is_a20gate_enabled:
    pusha

    mov ax, 0xFFFF
    mov es, ax

    xor ax, ax
    mov ds, ax

    ; Need to preserve what we are overwritting.
    push word [ds:0x500]
    push word [es:0x510]

    ; Write word 0x0000 at 0x0000:0x500 and 
    ;            0x0001 at 0xFFFF:0x510
    mov [ds:0x500], byte 0
    mov [es:0x510], byte 1

    ; Check if word written at location 0x0000:500 is still zero.
    cmp [ds:0x500], byte 0
    je .enabled

.disabled:
    clc
    jmp .end
.enabled:
    stc
.end:
    ; Need to preserve what we are overwritting.
    pop word [es:0x510]
    pop word [ds:0x500]
    popa
    ret

; If A20 gate is not enabled, then this routine will call the different methods 
; and try to enable A20 Gate. 
; Input:
;   None
; Output:
;   Carry   -  1  (A20 is enabled in the end)
;           -  0  (A20 is disabled in the end)
enable_a20:
    
    call __is_a20gate_enabled
    jc .enabled

    ; Method 1: BIOS method
    ;call __enable_a20gate_bios
    call __is_a20gate_enabled
    jc .enabled
    printString msg_failed

    ; Method 2: Fast Boot
    call __enable_a20_fast
    call __is_a20gate_enabled
    jc .enabled
    printString msg_failed

    ; Method 3: Keyboard controller

    ; Failed to enable
.failed:
    clc
    ret

.enabled:
    stc
    ret
