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
;  Build - 20201019
;  - Going to add a new GDT segment for a 128 KB kernel stack.
;  - The Stack will be placed at location 0x08000 offset.
;  - Segment size: 128 KB
;  
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

; Stack segment: 
; NOTE: We have to use Expand down segment for Stack if we want processor to
;       check the lower bound, and stack overflow.
; NOTE: 0x08000 to 0x27FFF    ->  128 KB
.stack: dw      0xFFFF      ; Limit
        dw      0x8000      ; Base
        db      0x00        ; Base
        db      0x92        ; Access: Data, 0 DPL, Expand up
        db      0x41        ; Limit: 0x01
                            ; Flags: 32Bit address, data, G = 1
        db      0x00        ; Base

;.stack: dw      0x8000      ; Limit
        ;dw      0x7FFF      ; Base
        ;db      0x01        ; Base
        ;db      0x96        ; Access: Data, 0 DPL, Expand down
        ;db      0x40        ; Limit: 0x01
                           ; Flags: 32Bit address, data, G = 0
        ;db      0x00        ; Base
.length equ $-gdt

gdt_meta:
    .size: dw  gdt.length-1                 ; Size of GDT -1
    .loc:  dd  (GDT_SEG * 0x10 + GDT_OFF)   ; 0x0000: 0x0800 = 0x0800

; ---------------------------------------------------------------------------
;  Code Segment
; ---------------------------------------------------------------------------

; ---------------------------------------------------------------------------
; Load the initial Global Descriptor Table into the GDTR register.
; The local GDT will be copied to a global location [ 0x0000: 0x0800 ]
; Input:
;   None
; Output:
;   None
load_gdt:
    push cx
    cli

    ; Copy GDT to globa location
    mov cx, GDT_SEG
    mov es, cx

    mov di, GDT_OFF
    mov cx, gdt.length
    mov si, gdt
    rep movsb
    
    ; Load gdtr
    lgdt [gdt_meta]

    sti
    pop cx
    ret
; ---------------------------------------------------------------------------

