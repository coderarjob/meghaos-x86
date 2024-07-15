; Megha Operating System V2 - x86 Kernel Entry
;
; Paging structure initialization and setting up of Higher-half kernel.
; ---------------------------------------------------------------------------

extern kernel_main
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
; Macros
; ---------------------------------------------------------------------------

; ---------------------------------------------------------------------------
; memset - Fills a memory location with a byte
; Input:
;   %0 - Destination location
;   %1 - Byte to fill with
;   %2 - Count
; Output:
;   None
; ---------------------------------------------------------------------------
%macro memset 3
    push eax
    push ecx
    push edi

    mov edi, %1
    mov eax, %2
    mov ecx, %3
    rep stosb

    pop edi
    pop ecx
    pop eax
%endmacro
; ---------------------------------------------------------------------------
; Text Section
; ---------------------------------------------------------------------------
section .prepage.text progbits alloc exec nowrite

g_kernel_entry:

    ; Save the static addresses to global variables
    mov [PHY(g_page_dir)], DWORD KERNEL_PAGE_DIR_MEM
    mov [PHY(g_page_table)], DWORD KERNEL_PAGE_TABLE_MEM
    mov [PHY(g_pab)], DWORD KERNEL_PAB_MEM

    ; Initialise page directory and table memories
    memset [PHY(g_page_dir)], 0, 4096
    memset [PHY(g_page_table)], 0, 4096

    ; Initialise the PD and PT
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

    ; Enable paging and Write Protection
    mov eax, cr0
    or eax, 0x80010000 ; PG, WP flags are set.
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

    xor ebp, ebp        ; Required for stack_trace to end here.
    jmp kernel_main
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
