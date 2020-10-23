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
; Bulid 20201019
; - GDT will always reside at location 0x0800. It is no longer at
;   Kernel's/boot1's local address.
; - Expand down kernel stack segment of 128 KB.
; - FS is also set to the Data Segment index.
; ---------------------------------------------------------------------------
; Bulid 20201008
; - Jump to kernel code, is done using
;       jmp <gdt seg>:dword <kernel_memory>
;   instead of the machine code.
; ---------------------------------------------------------------------------
; Version 20201006
; - GDT is local, not at 0x0000:0x0800. Kernel will have a separate GDT in its
;   local address space, and the one here will be deprecated, when kernel
;   loads.
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

    mov eax, 0x10    ; GDT index 2
    mov ds, eax
    mov es, eax
    mov gs, eax
    mov fs, eax
    mov ss, eax
    mov esp, KSTACK_TOP

    ; -------- [ Jumping to Kernel ] -----------
    jmp 0x8:dword (KERNEL_SEG * 0x10 + KERNEL_OFF)

.end:
    jmp $
.failed:
    printString msg_failed
    jmp .end

