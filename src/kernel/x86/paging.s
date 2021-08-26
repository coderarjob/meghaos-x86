; Megha Operating System V2 - x86 Kernel - Paging
; ---------------------------------------------------------------------------
; Paging structure initialization and setting up of Higher-half kernel.
;
; Dated: 18th December 2020
; ---------------------------------------------------------------------------
;  Change Log
; ---------------------------------------------------------------------------
; Build - 20201218
; - Intiial version. Identity paging implementation
; ---------------------------------------------------------------------------

extern __kernel_main
global __kernel_entry
global __page_dir
global __page_table

; ---------------------------------------------------------------------------
; Structures
; ---------------------------------------------------------------------------
MAX_FILES_COUNT  EQU 11
BOOT_INFO_MEM    EQU 0x1800

struc mem_des_t
            .BaseAddrLow   : resd 1
            .BaseAddrHigh  : resd 1
            .LengthLow     : resd 1
            .LengthHigh    : resd 1
            .Type          : resd 1
endstruc

struc file_des_t
            .StartLocation : resd 1
            .Length        : resw 1
endstruc

struc boot_info_t
            .file_count    : resw 1
            .file_dec_items: resb file_des_t_size * MAX_FILES_COUNT
            .mem_des_count : resw 1
            .mem_des_items : resb mem_des_t_size
endstruc

; ---------------------------------------------------------------------------
; Data Section
; ---------------------------------------------------------------------------
section .prepage.bss nobits alloc
__page_dir  : resd 1
__page_table: resd 1

; ---------------------------------------------------------------------------
; Text Section
; ---------------------------------------------------------------------------
section .prepage.text progbits alloc exec nowrite
__kernel_entry:

    ; Calculates based on the last file loaded by boot1, the
    ; 4KB aligned location to place page directory.
        call s_calculate_page_dir_location
        mov [__page_dir], eax
    ; --

    ; Page table is placed 4KB from the Page directory.
        add eax, 0x1000
        mov [__page_table], eax
    ; --

    xchg bx, bx
    call fill_pd
    call fill_pt

    xchg bx, bx
    mov eax, [__page_dir]
    and eax, 0b11111111_11111111_11110000_00000000
    mov cr3, eax
    
    mov eax, cr4
    and eax, 0b11111111_11111111_11111111_11101111
    mov cr4, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov eax, [__page_dir]
    and eax, 0b11111111_11111111_11110000_00000000
    mov cr3, eax

    xchg bx, bx
    jmp __kernel_main
    hlt

fill_pd:
    pushad
        mov edi, [__page_dir]
        mov eax, [__page_table]
        and eax, 0b11111111_11111111_11110000_00000000
        or eax, 7
        mov [edi],eax
        mov [edi + 768*4],eax
    popad
    ret

fill_pt:
    pushad
        mov edi, [__page_table]
        mov eax, 0
        mov ecx, 1024           ; There are 1024 entries per page table.
.write_next_pte:
        push eax
            and eax, 0b11111111_11111111_11110000_00000000
            or eax, 7
            mov [edi],eax
        pop eax
        add edi, 4
        add eax, 4096
        loop .write_next_pte
    popad
    ret

; ---------------------------------------------------------------------
; Calculates the locattion where to place the page directory.
; This address is located at the next 4 KB aligned location afer the
; last loaded file.
; Input:
;   None
; Ouptut:
;   EAX - Location where to place the page directory.
; ---------------------------------------------------------------------
s_calculate_page_dir_location:
    pushad
        xor  edi, edi
        mov  di , [BOOT_INFO_MEM + boot_info_t.file_count]
        dec  edi
        imul edi, file_des_t_size

        lea  edi, [BOOT_INFO_MEM + boot_info_t.file_dec_items + edi]
        mov  eax, [edi + file_des_t.StartLocation]
        add  ax , [edi + file_des_t.Length]
        adc  eax, 0                   ; Adds any carry from the previous ADD.

        ; Aligns the address to the next 4KB location.
        ; ADDR_ALIGNED_4KB = (INT(ADDR/4096) + 1) * 4096
            shr eax, 12
            inc eax
            shl eax, 12
        ; --

        mov [.ret_value], eax
    popad
    mov eax, [.ret_value]
    ret
.ret_value: dd 0
; ---------------------------------------------------------------------
