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
;  Build - 20201006
;  - GDT will reside in the DATA Section of the boot1 program instead of 
;    0x000:0x800. 
;    * This is done to prevent having two copies of GDT around, one
;      at local address space and another at 0:0x800. 
;    * Also it cannot be a great way of working, as any change in the 
;      local GDT, will result in copy to the global GDT. 
;
;      Having one copy, and it being in the local address space will be the 
;      standard in the whole kernel.
;    * Having a local GDT means that KENREL will have a separate GDT, separate
;      from the one here in boot1 program. And thats good!!
; ---------------------------------------------------------------------------
;  Build - 20200919
;  - Initial version. GDT at location 0x0000:0x0800
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
    .loc:  dd  gdt

; ---------------------------------------------------------------------------
;  Code Segment
; ---------------------------------------------------------------------------

; ---------------------------------------------------------------------------
; Load the initial Global Descriptor Table into the GDTR register.
; Input:
;   None
; Output:
;   None
load_gdt:
    ; Load gdt
    cli
    lgdt [gdt_meta]
    sti
    ret
; ---------------------------------------------------------------------------

