;
; Megha Operating System V2 - x86 Loader
; ---------------------------------------------------------------------------
; The main function of the loader, is to do setup the processor in protected
; mode with GDT, and load the kernel. 
; This program will use INT 0x31 for printing
;
; Note: The loader is a transient program, so no Interrupt routine, System
; calls must be placed in the loader.
;
; Dated: 8th September 2020
; ---------------------------------------------------------------------------
; Change Log
; ---------------------------------------------------------------------------
; Version 20201006
; - GDT is local, not at 0x0000:0x0800. Kernel will have a separate GDT in its
;   local address space, and the one here will be deprecated.
; ---------------------------------------------------------------------------
; Version 20200917
; - Welcome message
; - A20 Gate enabling
; ---------------------------------------------------------------------------
; Version 20200901
; - Initial version
; ---------------------------------------------------------------------------
    org 0x8000
    jmp _start
; ******************************************************
; MACRO BLOCK
; ******************************************************
%macro printString 1
    push si
    mov si, %1
    int 0x31
    pop si
%endmacro
; ******************************************************
; INCLUDE FILES
; ******************************************************
%include "a20gate.s"
%include "gdt.s"
%include "mos.inc"

; ******************************************************
; DATA
; ******************************************************
kernel_file: db     KERNEL_FILE

msg_welcome: db     "-------------------------------------"
             db     13,10,OS_NAME,13,10
             db     "boot0 : ",BOOT0_BUILD,","
             db     "boot1 : ",BOOT1_BUILD,
             db     13,10
             db     "-------------------------------------",0

msg_A20    : db 13,10,"A20 GATE","... ",0
msg_GDT    : db 13,10,"GDT","... ",0
msg_PMODE  : db 13,10,"Protected Mode","... ",0
msg_LD_KRNL: db 13,10,"Loading kernel image","... ",0
msg_ST_KRNL: db 13,10,"Starting kernel","... ",0

msg_success:  db "OK",0
msg_failed :  db "!",0

; ******************************************************
; CODE
; ******************************************************
_start:
    ; Welcome message
    printString msg_welcome

    ; -------- [ Load Kernel Image ] -----------
    printString msg_LD_KRNL

    mov ax, KERNEL_SEG
    mov bx, KERNEL_OFF
    mov dx, kernel_file

    int 0x30
    cmp ax, 0			            ; Check if read was successful
    je .failed	                    ; Show error message if read failed.

    printString msg_success

    ; -------- [ A20 Gate Enabling ] -----------
    printString msg_A20
    call enable_a20
    jnc .failed
    printString msg_success

    ; -------- [ GDT ] -----------
    printString msg_GDT
    call load_gdt
    printString msg_success

    ; -------- [ Enter Protected mode ] -----------
    printString msg_PMODE
    printString msg_success

    ; Say goodbye to 16 bits
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Clear prefetch queue
    jmp .clear_prefetch_queue
    nop
    nop
.clear_prefetch_queue:

    ; Disable Interrupts as IVT is not yet present
    cli

    mov ax, 0x10
    mov ds, eax
    mov es, eax
    mov ss, eax

    ; Kernel is at 0x18000, stack at 0x30000 -> 60K
    ; TODO: Find a better place for the kernel stack. 
    ; NOTE: Should we have LDT to prevent stack crashing into kernel data ?
    mov esp, 0x30000    
    

    ; -------- [ Jumping to Kernel ] -----------
    xchg bx, bx
    db  0x66
    db  0xea
    dd  0x18000
    dw  0x0008

.end:
    jmp $
.failed:
    printString msg_failed
    jmp .end

