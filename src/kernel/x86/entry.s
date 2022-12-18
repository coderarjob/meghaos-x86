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
global g_kernel_entry
global g_page_dir
global g_page_table
global g_pab

%include "mos.inc"
; ---------------------------------------------------------------------------
; Data Section
; ---------------------------------------------------------------------------
section .bss nobits alloc
g_page_dir:   resd 1
g_page_table: resd 1
g_pab:        resd 1

; ---------------------------------------------------------------------------
; Text Section
; ---------------------------------------------------------------------------
section .prepage.text progbits alloc exec nowrite

g_kernel_entry:

    ; Calculates based on the last file loaded by boot1, the
    ; 4KB aligned location to place page directory.
        call s_calculate_page_dir_location
        mov [PHY(g_page_dir)], eax
    ; --

    ; Page table is placed 4KB from the Page directory.
        add eax, 0x1000
        mov [PHY(g_page_table)], eax

    ; Page allocation bitmap is placed after the Page table.
        add eax, 0x1000
        mov [PHY(g_pab)], eax
    ; --

    call s_fill_pd
    call s_fill_pt

    ; Caching is enabled, with write-back caching.
    mov eax, [PHY(g_page_dir)]
    and eax, 0b11111111_11111111_11110000_00000000
    mov cr3, eax
    
    ; Page size extension and page address extension is disabled
    mov eax, cr4
    and eax, 0b11111111_11111111_11111111_11001111
    mov cr4, eax

    ; Enable paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; The below .text portion will get linked at a different location
    ; in memory. We need to jump to it.
    jmp .higher_mapped

; ---------------------------------------------------------------------
    ; Before we can remove the identity mapping, the symbols
    ; need to have higher addresses. That is why the below
    ; source is in .text section just like the rest of the
    ; kernel. This portion will actually be linked after the
    ; whole to .prepage.text. So will not be in the order it
    ; appears here in the source.
section .text progbits alloc exec nowrite

.higher_mapped:
    ; Disable identity map
       mov eax, [g_page_dir]
       mov [eax], dword 0
    ; --

    ; Map stack to higher address
    mov esp, HIGHV(KERNEL_STACK_TOP_MEM)

    ; Clear TLB
        mov eax, [g_page_dir]
        mov cr3, eax
    ; --

    jmp __kernel_main
    hlt
; ---------------------------------------------------------------------

section .prepage.text progbits alloc exec nowrite

; ---------------------------------------------------------------------
; Sets up the Page directory to have both identity mapping and mapping
; at 0xC0000000 (3 GB) address.
; The same page table is mapped at two locations in the page directory
; This will allow the same range of physical address to be available
; via two distinct virtual addresses.
; Input:
;   None
; Ouptut:
;   None
; ---------------------------------------------------------------------
s_fill_pd:
    pushad
        mov edi, [PHY(g_page_dir)]
        mov eax, [PHY(g_page_table)]
        ; Caching is enabled, with write-back caching.
        and eax, 0b11111111_11111111_11110000_00000000
        or eax, 7
        mov [edi],eax           ; Identity map
        mov [edi + 768*4],eax   ; Higher-half map

        ; Recursive maping
        ; Caching is enabled, with write-back caching.
        mov eax, [PHY(g_page_dir)]
        and eax, 0b11111111_11111111_11110000_00000000
        or eax, 7
        mov [edi + 1023*4], eax
    popad
    ret

; ---------------------------------------------------------------------
; Sets up the page table to map address 0x000000 to 0x3FFFFF
; Input:
;   None
; Ouptut:
;   EAX - Location where to place the page directory.
; ---------------------------------------------------------------------
s_fill_pt:
    pushad
        mov edi, [PHY(g_page_table)]
        mov eax, 0
        mov ecx, MIN_MEM_REQ / 4096    ; Assuming the minimum ram available.
.write_next_pte:
        push eax
            ; Caching is enabled, with write-back caching.
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
