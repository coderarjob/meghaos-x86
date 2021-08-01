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

section .prepage.bss nobits alloc
align 4096
__page_dir:
    resd 1024
__page_table:
    resd 1024

section .prepage.text progbits alloc exec nowrite
__kernel_entry:

    xchg bx, bx
    call fill_pd
    call fill_pt

    xchg bx, bx
    mov eax, __page_dir
    and eax, 0b11111111_11111111_11110000_00000000
    mov cr3, eax
    
    mov eax, cr4
    and eax, 0b11111111_11111111_11111111_11101111
    mov cr4, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov eax, __page_dir
    and eax, 0b11111111_11111111_11110000_00000000
    mov cr3, eax

    xchg bx, bx
    jmp __kernel_main
    hlt

fill_pd:
    pushad
        mov edi, __page_dir 
        mov eax, __page_table
        and eax, 0b11111111_11111111_11110000_00000000
        or eax, 7
        mov [edi],eax
        mov [edi + 768*4],eax
    popad
    ret

fill_pt:
    pushad
        mov edi, __page_table
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
