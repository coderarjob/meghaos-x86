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
; Dated: 25 Aug 2021
; ---------------------------------------------------------------------------
    org 0x8000
    jmp _start
; ******************************************************
; MACRO BLOCK
; ******************************************************
%macro printString 1-2 0
    push si
    lea si, [%1 + %2]
    int 0x31
    pop si
%endmacro

; ******************************************************
; INCLUDE FILES
; ******************************************************
%include "a20gate.s"
%include "gdt.s"
%include "vbe2.s"
%include "mos.inc"

; ******************************************************
; DATA
; ******************************************************
files:       db     KERNEL_FILE     , "KERNEL.FLT",0,0,0
%ifdef GRAPHICS_MODE_ENABLED
             db     GUI0_APP_FILE   , "GUI0.FLT",0,0,0,0,0
             db     MOS_IMAGE_FILE  , "MOS.RBM",0,0,0,0,0,0
%else
             db     PROC1_FILE      , "PROC1.FLT",0,0,0,0
             db     MPDEMO_FILE     , "MPDEMO.FLT",0,0,0
%endif
             db     INIT_FILE       , "INIT.FLT",0,0,0,0,0
             db     0

msg_welcome: db     13,10,OS_NAME,13,10
             db     "  boot0 : ",BOOT0_BUILD,","
             db     "  boot1 : ",BOOT1_BUILD,0

msg_A20    : db 13,10,"[  ]    A20 GATE. ",0
msg_GDT    : db 13,10,"[  ]    GDT. ",0
msg_PMODE  : db 13,10,"[  ]    Protected mode. ",0
msg_LD_FILE: db 13,10,"[  ]    Loading file : ",0
msg_ST_KRNL: db 13,10,"[  ]    Starting kernel. ",0
msg_MEMINFO: db 13,10,"[  ]    BIOS memory info. ",0
msg_AVLMEM : db 13,10,"[  ]    Available memory. ",0

msg_success: db 13,"[OK]",0
msg_failed : db 13,"[ER]",0

%ifdef GRAPHICS_MODE_ENABLED
vbemode:
    ; Note: In NASM Version < 2.16, AT keyword requires complete field names.
    ; Version 2.16 fixes this saying "Fixed the handling AT in istruc when the
    ; structure contains local labels."
    ; Since the minimum version we support is NASM 2.15.05, the struc field
    ; names need to be complete.
    istruc vbe_modequery_t
        ; Input to vbe2_find_mode, vbe2_switch_mode
        at vbe_modequery_t.Xresolution       , dw GXMODE_XRESOLUTION
        at vbe_modequery_t.Yresolution       , dw GXMODE_YRESOLUTION
        at vbe_modequery_t.BitsPerPixel      , db GXMODE_BITSPERPIXEL

        ; Output from vbe2_find_mode
        ; Input for vbe2_switch_mode
        at vbe_modequery_t.Mode              , dw 0
        at vbe_modequery_t.VbeVersion        , dw 0
        at vbe_modequery_t.FrameBuffer       , dd 0
        at vbe_modequery_t.BytesPerScanLine  , dw 0
    iend
%endif

; ******************************************************
; CODE
; ******************************************************
_start:
    ; -------- [ Set 9x8 font  ] -----------
    mov bl, 0x00
    mov ax, 0x1112
    int 0x10

    ; -------- [ Welcome message ] -----------
    printString msg_welcome

    ; -------- [ DEBUG: Print VBE Information] -----------
%ifdef GRAPHICS_MODE_ENABLED
    call vbe2_dump_modes
%endif

    ; -------- [ Clean Boot info memory ] -----------
    mov al, 0
    mov bx, BOOT_INFO_SEG
    mov es, bx
    mov di, BOOT_INFO_OFF
    mov cx, boot_info_t_size
    rep stosb

    ; -------- [ Get memory information ] -----------
    printString msg_MEMINFO
    call __get_mem_info
    printString msg_success
    
    ; -------- [ Calculate total usable memory ] -----------
    ; Check for minimum memory. Halts if amount is less.
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

    ; Check if the amount of free memory is >= Minimum Required RAM
    cmp eax, HIGH_BITS(MIN_MEM_REQ,32)
    ja .ne1                                       ; HIGH(AVRAM) > HIGH(MIN REQ)
    jb failed                                     ; HIGH(AVRAM) < HIGH(MIN REQ)
    cmp ebx, LOW_BITS(MIN_MEM_REQ,32)             ; HIGH(AVRAM) = HIGH(MIN REQ)
    jb failed                                     ; LOW(AVRAM) < LOW(MIN REQ)
.ne1:                                             ; AVRAM >= MIN REQ RAM
    printString msg_success

    ; -------- [ A20 Gate Enabling ] -----------
    printString msg_A20
    call enable_a20
    jnc failed
    printString msg_success

    ; -------- [ Load Kernel and ramdisk files ] -----------
    call __load_kernel_and_ramdisks

    ; -------- [ Enter Protected Mode and Jump to Kernel ] -----------
    ; TODO: Do we really need to copy the GDT to a global location.
    ;       The location of the GDT can be passed to the kernel in the
    ;       BOOT_INFO structure. And the kernel can do whatever it wants
    ;       with it.
    printString msg_GDT
    printString msg_success

    printString msg_PMODE
    printString msg_success

    call copy_gdt_to_global

    ; -------- [ Switch to graphics Mode ] -----------
%ifdef GRAPHICS_MODE_ENABLED
    mov edi, vbemode
    call vbe2_find_mode
    jc .gx_failed

    mov edi, vbemode
    call vbe2_switch_mode
    jc .gx_failed

    ; Graphics mode was set successfully

    ; ----  Copy the graphics mode info into Boot info
    mov cx, vbe_modequery_t_size ; Number of bytes to copy
    mov bx, BOOT_INFO_SEG
    mov es, bx                   ; Destination (ES:DI)
    lea di, [BOOT_INFO_OFF + boot_info_t.graphis_info]
    mov si, vbemode              ; Source (DS:SI)
    rep movsb

    ; ----  Copy fonts data from BIOS memory to BOOT_INFO
    mov ax, 0x1130
%if GXMODE_FONT_WIDTH == 8 && GXMODE_FONT_HEIGHT == 16
    mov bh, 6 ; 8 x 16 font (May be only for VGA)
%else
    %error "Invalid glyph size"
%endif
    int 0x10 ; Returns font data in ES:BP

    push ds
        push es ; Move ES to DS
        pop ds
        mov si, bp ; Source (DS:SI)

        mov cx, GXMODE_FONTS_DATA_BYTES ; Number of bytes to copy
        lea di, [BOOT_INFO_OFF + boot_info_t.fonts_data]
        mov bx, BOOT_INFO_SEG
        mov es, bx           ; Destination (ES:DI)
        rep movsb
    pop ds

    ; Copy complete, continue loading kernel
    jmp .goto_kernel

.gx_failed:
    ; Clear Mode to indicate failure. This  also means that OS is operating in
    ; text mode
    mov [vbemode + vbe_modequery_t.Mode], word 0
%endif

    ; -------- [ All set, now jump to kernel ] -----------
.goto_kernel:
    EnterProtectedMode32 gdt32_meta_global
    [BITS 32]
    jmp KERNEL_IMG_MEM
    [BITS 16]
.end:
    jmp $


; Loads Kernel image and Ram Disk files to RAM.
; Input:
;   None
; Output:
;   Success - None
;   Failure - Will jump to 'finish' and halt.
__load_kernel_and_ramdisks:
    pusha
     ; Points to current file.
     mov si, files                                      

    ; Points into file_des_items[] array
    lea di, [BOOT_INFO_OFF + boot_info_t.file_dec_items]

    ; Counts the number of loaded files.
    xor ecx, ecx
.load_next:

    ; Print the file name on screen
        printString msg_LD_FILE
        printString si, 11

    ; --- Load the current file into temprary buffer.
        mov ax, FILE_BUFF_SEG
        mov bx, FILE_BUFF_OFF
        mov dx, si
        int 0x30
        cmp ax, 0			            ; Check if read was successful
        je failed	                    ; Show error message if read failed.

    ; ---- Copy the buffer, into its proper location
        EnterProtectedMode32
        [BITS 32]

        pusha
            xor ecx, ecx
            mov cx, ax

            mov ebx, 0x10    ; GDT index 2
            mov es, ebx
            mov ds, ebx

            mov esi, FILE_BUFF_MEM
            mov edi, [.copy_dest_location]

            rep movsb
        popa
         
        EnterProtectedMode16
        [BITS 16]
        EnterRealMode
    ; ---- Update the file_des_items array and file_count in boot_info
        pusha
            mov bx, BOOT_INFO_SEG
            mov es, bx

            mov [es:di + file_des_t.Length], ax
            mov eax, [.copy_dest_location]
            mov [es:di + file_des_t.StartLocation], eax

            ; --- Copy file name
            pusha
                lea di, [es:di + file_des_t.name]
                mov cx, 11
                rep movsb

                ; Add a zero at the end to form a zero terminated string
                mov [di], byte 0;
            popa

            inc word [es:BOOT_INFO_OFF + boot_info_t.file_count]
        popa

        printString msg_success

    ; ---- Next file name after
        ; Increment Destination location by the value in EAX. The high bits of
        ; EAX can be garbage so we clear it before adding.
        push ax
            xor eax, eax
        pop ax
        add [.copy_dest_location], eax
        add di, file_des_t_size
        inc cx

        ; We have loaded MAX_FILES_COUNT. We load no more!
        cmp cx, MAX_FILES_COUNT
        je .nomore

        add si, 24
        cmp [si], byte 0
        jnz .load_next
.nomore:
    popa
    ret
.copy_dest_location  : dd KERNEL_IMG_MEM
.count_completed     : dw 0

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

; Helper routine for the __e820 routine, which sets up the boot_info_t 
; structure in BOOT_INFO segment.
; Input:
;   None
; Output:
;   Success:
;     ES:DI   - Points to the BOOT_INFO structure location.
;     AX      - Number of entries of entries.
;   Failure:
;             - Jumps to 'finish' and Halts.
__get_mem_info:
    
    ; Clears the count
    mov ax, BOOT_INFO_SEG
    mov es, ax
    mov [es:BOOT_INFO_OFF + boot_info_t.mem_des_count], word 0

    ; Fill the mem map in the boot_info_t structure.
    lea di, [BOOT_INFO_OFF + boot_info_t.mem_des_items]
    call __e820

    ; Write the returned count in boot_info_t structure
    ; NOTE: MOV does not effect any flag. So no need to preserve EFLAGS.
    mov [es:BOOT_INFO_OFF + boot_info_t.mem_des_count], ax
.fin:
    ret

; Calls BIOS routine INT 15H (EAX = 0xE820) to get MemoryMap
; NOTE: There is no maximum limit of the number of entries that 
;       we want in the array. May lead to overwriting/overlaping 
;       when kernel is loaded.
; Input: 
;       ES:DI - Location to where the array of mem_des_t will be saved.
; Output: 
;   Success:
;       AX      - Number of entries of entries. (No error)
;   Failure:
;               - Jumps to 'finish' and Halts.
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

    jc failed                   ; CR = 1, means error.
    test ebx, ebx               ; EBX = 0, means last entry.
    jnz .loop                   ; Loop if not last.
.success:
    clc                         ; CF = 0 (not error)
    mov ax, [.mem_des_count]    ; Returns entry count.

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

; Global jump location when there is an unrecovarable error.
failed:
    printString msg_failed
    jmp $
