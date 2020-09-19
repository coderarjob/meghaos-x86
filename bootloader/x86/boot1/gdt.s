;
; Megha Operating System V2 - x86 GDT
; ---------------------------------------------------------------------------
;  The main function of the loader, is to do setup the processor in protected
;  mode with GDT, and load the kernel. 
;  This program will use INT 0x31 for printing
;
;  Note: The loader is a transient program, so no Interrupt routine, System
;  calls must be placed in the loader.
;
;  Dated: 18th September 2020
; ---------------------------------------------------------------------------
;  Change Log
; ---------------------------------------------------------------------------
;  Build - 20200919
;  - Initial version
; ---------------------------------------------------------------------------

; ---------------------------------------------------------------------------
;  Included files
; ---------------------------------------------------------------------------
%include "mos.inc"

; ---------------------------------------------------------------------------
;  Data Section
; ---------------------------------------------------------------------------

gdt:    
.null:  times 8 db 0
.code:  dw      0xFFFF      ; Limit
        dw      0x0000      ; Base
        db      0x00        ; Base
        db      0x9A        ; Access: System Descriptor, Code, 0 DPL, NC
        db      0xCF        ; Limit: 0xFF, 
                            ; Flags: 32Bit address, data, 4GB limit.
        db      0x00        ; Base

.data:  dw      0xFFFF      ; Limit
        dw      0x0000      ; Base
        db      0x00        ; Base
        db      0x92        ; Access: Data, 0 DPL
        db      0xCF        ; Limit: 0xFF, 
                            ; Flags: 32Bit address, data, 4GB limit.
        db      0x00        ; Base
.length equ $-gdt

gdt_meta:
    .size: dw  gdt.length-1        ; Size of GDT -1
    .loc:  dd  GDT_OFF             ; GDT is stored at 0x0000:0x0800

; ---------------------------------------------------------------------------
;  Code Segment
; ---------------------------------------------------------------------------

; ---------------------------------------------------------------------------
; Load the initial Global Descriptor Table into 0x000:0x800 location, and loads
; the GDTR register.
; The 0x800 location is chosen because that is where the IDT ends.
; ---------------------------------------------------------------------------
; Input:
;   None
; ---------------------------------------------------------------------------
; Output:
;   None
; ---------------------------------------------------------------------------
load_gdt:
    pusha 
    ; Copy the GDT table from boot1 memory to 0x000:0x800
    xor ax, ax
    mov es, ax
    mov di, 0x800
    mov si, gdt

    mov cx, gdt.length
    rep movsb

    ; Load gdt
    lgdt [gdt_meta]

    popa
    ret

