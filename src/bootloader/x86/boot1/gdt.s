;
; Megha Operating System V2 - x86 GDT
; ---------------------------------------------------------------------------
;  The main function of the loader, is to do setup the processor in protected
;  mode with GDT, and load the kernel. 
;  This program will use INT 0x31 for printing
;
;  Note: The loader is a transient program, so no Interrupt routine, System
;  calls must be placed in the loader.
; ---------------------------------------------------------------------------

; ---------------------------------------------------------------------------
;  Included files
; ---------------------------------------------------------------------------
%include "mos.inc"
; ---------------------------------------------------------------------------
;  Data Section
; ---------------------------------------------------------------------------
gdt32:    
.null:  times 8 db 0
.code:  dw      0xFFFF      ; Limit
        dw      0x0000      ; Base
        db      0x00        ; Base
        db      0x9A        ; Access: System Descriptor, Code, 0 DPL, NC
        db      0xCF        ; Limit: 0xFF, 
                            ; Flags: 32Bit address, data, 4GB limit.
        db      0x00        ; Base

; Stack and data segment for the kernel.
; Stack will be set to reside from 0x27FFF. Cannot assign separate segment for
; stack without as GCC does not allow SS =/= DS.
.data:  dw      0xFFFF      ; Limit
        dw      0x0000      ; Base
        db      0x00        ; Base
        db      0x92        ; Access: Data, 0 DPL
        db      0xCF        ; Limit: 0xFF, 
                            ; Flags: 32Bit address, data, 4GB limit.
        db      0x00        ; Base

.length equ $-gdt32

gdt32_meta:
    .size: dw  gdt32.length-1                 ; Size of GDT -1
    .loc:  dd  gdt32

gdt32_meta_global:
    .size: dw  gdt32.length-1                 ; Size of GDT -1
    .loc:  dd  (GDT_SEG << 4 | GDT_OFF)
; ---------------------------------------------------------------------------
gdt16:    
.null:  times 8 db 0

.code:  dw      0xFFFF      ; Limit
        dw      0x0000      ; Base
        db      0x00        ; Base
        db      0x9A        ; Access: System Descriptor, Code, 0 DPL, NC
        db      0x10        ; Limit: 0xFF, 
                            ; Flags: 32Bit address, data, 4GB limit.
        db      0x00        ; Base

.data:  dw      0xFFFF      ; Limit
        dw      0x0000      ; Base
        db      0x00        ; Base
        db      0x92        ; Access: Data, 0 DPL
        db      0x10        ; Limit: 0xFF, 
                            ; Flags: 32Bit address, data, 4GB limit.
        db      0x00        ; Base
.length equ $-gdt16

gdt16_meta:
    .size: dw  gdt16.length-1                 ; Size of GDT -1
    .loc:  dd  gdt16
; ---------------------------------------------------------------------------
;  Macro definations
; ---------------------------------------------------------------------------
%macro EnterProtectedMode32 0-1 gdt32_meta
    push eax
        ; Load GDT
        lgdt [%1]
        
        ; Setup CR0
        mov eax, cr0
        or eax, 1
        mov cr0, eax
        
        jmp %%.clear_prefetch_queue
        nop
        nop
%%.clear_prefetch_queue:
    
        ; Note: For some reason an explicit CLI is required here.
        ;       CLI at the very beginning do not work it seems!
        cli

        ; Setup segment registers
        mov eax, 0x10
        mov ds, eax
        mov es, eax
        mov gs, eax
        mov fs, eax
        mov ss, eax
    
    pop eax

    ; Jump to 32 bit protected mode
    jmp 0x8:%%.pmode32
%%.pmode32:

%endmacro

%macro EnterProtectedMode16 0
    push eax

        ;Load GDT
        lgdt [gdt16_meta]

        jmp %%clear_prefetch_queue
        nop
        nop
%%clear_prefetch_queue:

        ; Note: For some reason an explicit CLI is required here.
        ;       CLI at the very beginning do not work it seems!
        cli

        ; Setup segment registers
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov gs, ax
        mov fs, ax
        mov ss, ax

    pop eax

    ; Jump to 16 bit protected mode
    jmp 0x8:%%pmode16
%%pmode16:
%endmacro

%macro EnterRealMode 0
    push eax

        ; Disable Protected mode
        mov eax, cr0
        and eax, ~1
        mov cr0, eax

        jmp 0:%%realmode
%%realmode:

        ; Setup segment registers
        mov ax, 0
        mov ds, ax
        mov es, ax
        mov gs, ax
        mov fs, ax
        mov ss, ax
   
   pop eax
%endmacro
; ---------------------------------------------------------------------------
;  Code Segment
; ---------------------------------------------------------------------------

; Copy the initial Global Descriptor Table to a global location.
; The local GDT will be copied to a global location [ 0x0000: 0x0800 ]
; Input:
;   None
; Output:
;   None
copy_gdt_to_global:
    pusha
    push es

    mov cx, GDT_SEG
    mov es, cx
    mov di, GDT_OFF

    mov cx, gdt32.length
    mov si, gdt32
    rep movsb

    pop es
    popa
    ret
; ---------------------------------------------------------------------------

