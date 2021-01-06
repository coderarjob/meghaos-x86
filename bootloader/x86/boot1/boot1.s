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

%define HIGH(x,b) ((x)>>(b))
%define LOW(x,b) ((x) & (x^b-1))

; ******************************************************
; INCLUDE FILES
; ******************************************************
%include "a20gate.s"
%include "gdt.s"
%include "mos.inc"

; ******************************************************
; DATA
; ******************************************************
struc mem_des_t
    .BaseAddrLow   : resd 1
    .BaseAddrHigh  : resd 1
    .LengthLow     : resd 1
    .LengthHigh    : resd 1
    .Type          : resd 1
endstruc

struc boot_info_t
    .mem_des_count : resw 1
    .mem_des_items : resw mem_des_t_size
endstruc

kernel_file: db     KERNEL_FILE

msg_welcome: db     13,10,OS_NAME,13,10
             db     "  boot0 : ",BOOT0_BUILD,","
             db     "  boot1 : ",BOOT1_BUILD,0

msg_A20    : db 13,10,"A20 GATE... ",0
msg_GDT    : db 13,10,"GDT... ",0
msg_PMODE  : db 13,10,"Protected Mode... ",0
msg_LD_KRNL: db 13,10,"Loading kernel image... ",0
msg_ST_KRNL: db 13,10,"Starting kernel... ",0
msg_MEMINFO: db 13,10,"BIOS memory info... ",0
msg_AVLMEM : db 13,10,"Available memory... ",0

msg_success:  db " OK",0
msg_failed :  db " !",0

; ******************************************************
; CODE
; ******************************************************
_start:
    ; -------- [ Welcome message ] -----------
    printString msg_welcome

    ; -------- [ Get memory information ] -----------
    printString msg_MEMINFO
    call __get_mem_info
    jc .failed
    printString msg_success
    
    ; -------- [ Calculate total usable memory ] -----------
    ; We require at least 4 MB of usable RAM. We HALT if the amount is less.
    printString msg_AVLMEM
    ; AX = Entry Count
    ; ES:DI = Pointer to mem_des_t array
    ; Return in EAX:EBX
    call __calc_total_mem

    ; Display the total free memory in hex
    call __printhex         ; Display HIGH (EAX)
    push eax                ; Display LOW (EBX)
        mov eax, ebx
        call __printhex
    pop eax

    ; Check if the amount of free memory is >= 4MB
    cmp eax, HIGH(MEM_AVL_MIN,32)                  ; 4 MiB
    jg .ne1                                        ; HIGH(AVRAM) > HIGH(4MB)
    jl .failed                                     ; HIGH(AVRAM) < HIGH(4MB)
    cmp ebx, LOW(MEM_AVL_MIN,32)                   ; HIGH(AVRAM) = HIGH(4MB)
    jl .failed                                     ; LOW(AVRAM) < LOW(4MB)
.ne1:                                              ; AVRAM >= 4MB
    printString msg_success

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

; Sums up the 'FREE' regions in the BOOT_INFO array.
; Input:
;       ES:DI - Location to where the array of mem_des_t will be saved.
;       AX    - Count
; Output:
;       EAX:EBX - 64bit sum
__calc_total_mem:
    push cx
    push di

    ; Clear the result 
    mov [.sum], dword 0
    mov [.sum + 2], dword 0

   ; CX holds the entry count
   mov cx, ax
   jcxz .fin

.again:
    ; Check if entry is of Type 'FREE' 
    cmp dword [es:di + mem_des_t.Type], 1
    jne .next

    ; Type = 1, We add with the SUM
    mov eax, [es:DI + mem_des_t.LengthLow]
    add [.sum], eax             ; Little endian, Low DWORD is at low address

    mov ebx, [es:DI + mem_des_t.LengthHigh]
    adc [.sum + 4], ebx

.next:
    add di, mem_des_t_size      ; Pointer to next entry
    loop .again

    ; Loop ended
.fin:
    mov ebx, [.sum]             ; Lower DWORD
    mov eax, [.sum + 4]         ; Higher DWORD

    pop di
    pop cx
    ret

.sum: dd 0      ; Stores the 64Bit sum
      dd 0

; Calls __e820 routine, and setup the boot_info_t structure in BOOT_INFO
; segment.
; Input:
;   None
; Output:
;   CR   - 1 (error)
;   CR   - 0 (no error)
__get_mem_info:
    
    ; Clears the count
    mov [es:BOOT_INFO_OFF + boot_info_t.mem_des_count], word 0

    ; Fill the mem map in the boot_info_t structure.
    mov ax, BOOT_INFO_SEG
    mov es, ax
    lea di, [BOOT_INFO_OFF + boot_info_t.mem_des_items]
    call __e820

    ; Write the returned count in boot_info_t structure
    ; NOTE: MOV does not effect any flag. So no need to preserve EFLAGS.
    mov [es:BOOT_INFO_OFF + boot_info_t.mem_des_count], ax
.fin:
    ret

; Calls BIOS routine INT 15H (EAX = 0xE820) to get MemoryMap
; NOTE: There is not maximum limit of the number of entries that 
;       we want in the array. May lead to overwriting/overlaping 
;       when kernel is loaded.
; Input: 
;       ES:DI - Location to where the array of mem_des_t will be saved.
; Output: 
;       CR flag - 1 (Error)
;       CR flag - 0 (No error, output in CX)
;       AX - Number of entries of entries.
__e820:
    push ebx
    push edx
    push ecx
    push di

    ; Continuation value starts from 0.
    xor ebx, ebx
    jmp .int15
.loop:
    add di, mem_des_t_size      ; Increment DI
    inc word [.mem_des_count]   ; Increment entry count
.int15:
    mov ecx, mem_des_t_size
    mov edx, 'PAMS'         ; 53 4D 41 50
    mov eax, 0xE820                 
    int 0x15

    jc .failed                  ; CR = 1, means error.
    test ebx, ebx               ; EBX = 0, means last entry.
    jnz .loop                   ; Loop if not last.
.success:
    clc                         ; CF = 0 (not error)
    mov ax, [.mem_des_count]    ; Returns entry count.
    jmp .fin            
.failed:
    stc                 ; Error in 0xE820 or it is not supported.
.fin:
    pop di
    pop ecx
    pop edx
    pop ebx
    ret
.mem_des_count: dw  1   ; Valid array length

; Prints 32 bit hex, on screen
; Input:
;   EAX
; Output:
;   None
__printhex:
    pushad

    mov ecx, 8
.next:
    mov ebx, eax
    and ebx, 0xF
    mov bl, [.hexchars + ebx]       ; nibble to hex character.
    mov [.hexstr + ecx - 1], bl     ; 8th nibble goes to index 7 of .hexstr
    shr eax, 4
    loop .next

    printString .hexstr
    popad
    ret
.hexchars: db "0123456789ABCDEF"
.hexstr: times 9 db 0               ; one extra zero in the end.

