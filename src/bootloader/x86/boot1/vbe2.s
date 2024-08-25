; Megha Operating System V2 - x86 VESA VBE2.0 driver
; ---------------------------------------------------------------------------

struc vbe2_infoblock_t
    .VbeSignature      : resb 4   ; Vesa Signature
    .VbeVersion        : resw 1   ; VBE Version in BCD
    .OemStringPtr      : resd 1   ; Far pointer to OEM String
    .Capabilities      : resb 4   ; VGA capabilities supported.
    .VideoModePtr      : resd 1   ; Pointer to list of video modes.
    .TotalMemory       : resw 1   ; Number of 64KB memory blocks
    .OemSoftwareRev    : resw 1   ; VBE implementation Software revision
    .OemVendorNamePtr  : resd 1   ; Pointer to vendor name string
    .OemProductNamePtr : resd 1   ; Pointer to product name string
    .OemProductRevPtr  : resd 1   ; Pointer to product revision string
    .reserved          : resb 222 ; Reserved: VBE implementation scratch area
    .OemData           : resb 256 ; Reserved: Data Area for OEM Strings
endstruc

struc vbe2_modeinfoblock_t
    .ModeAttributes      : resw 1
    .WinAAttributes      : resb 1
    .WinBAttributes      : resb 1
    .WinGranularity      : resw 1
    .WinSize             : resw 1
    .WinASegment         : resw 1
    .WinBSegment         : resw 1
    .WinFuncPtr          : resd 1
    .BytesPerScanLine    : resw 1

    .Xresolution         : resw 1
    .Yresolution         : resw 1
    .XCharSize           : resb 1
    .YCharSize           : resb 1
    .NumberOfPlanes      : resb 1
    .BitsPerPixel        : resb 1
    .NumberOfBanks       : resb 1
    .MemoryModel         : resb 1
    .BankSize            : resb 1   ; in KB
    .NumberofImagePages  : resb 1
    .reserved0           : resb 1

    .RedMaskSize         : resb 1
    .RedFieldPosition    : resb 1
    .GreenMaskSize       : resb 1
    .GreenFieldPosition  : resb 1
    .BlueMaskSize        : resb 1
    .BlueFieldPosition   : resb 1
    .RsvdMaskSize        : resb 1
    .RsvdFieldPosition   : resb 1
    .DirectColorModeInfo : resb 1

    .PhysBasePtr         : resd 1   ;  Physical address for flat memory fb
    .OffScreenMemOffset  : resd 1   ;  Pointer to start of off screen memory
    .OffScreenMemSize    : resw 1   ;  Number of 1k memory blocks
    .reserved1           : resb 206 ;  Number of 1k memory blocks
endstruc

; Prints null terminated string pointed to by the address of the form (high 16
; bits :- segment, low 16 bits :- offset)
%macro printVbeStrings 1
    push eax
    push ebx

    mov eax, %1; just store it in eax
    mov bx, ax ; bx now has the offset
    shr eax, 16; ax now has the segment

    ; printString expects short pointer with segment in DS register
    push ds
        mov ds, ax
        printString bx
    pop ds
    pop ebx
    pop eax
%endmacro

; ---------------------------------------------------------------------------
; Searches for the VBE modes and dumps the mode number and other information.
; Input:
;   None
; Output (Success):
;   None
; Output (Failue):
;   None
; ---------------------------------------------------------------------------
vbe2_dump_modes:
    pushad
    push es
    push gs

    push ds     ; Mov es, ds
    pop es

    ; Query the video modes
    push edi
        mov ax, 0x4F00
        mov di, vbeblock ; Location is ES:DI
        int 0x10
    pop edi

    ; AL != 0x4F means command is not supported
    cmp al, 0x4F
    jnz .query_failure

    ; Non-zeo AH means there is an error
    cmp ah, 00
    jnz .query_failure

    ; Print some information from the vbe block
    printString .msg_newline

    xor  eax, eax
    mov ax, [vbeblock + vbe2_infoblock_t.VbeVersion]
    call __printhex
    printString .msg_newline

    mov eax, [vbeblock + vbe2_infoblock_t.OemStringPtr]
    printVbeStrings eax
    printString .msg_newline

    xor  eax, eax
    mov eax, [vbeblock + vbe2_infoblock_t.Capabilities]
    call __printhex
    printString .msg_space

    xor  eax, eax
    mov ax, [vbeblock + vbe2_infoblock_t.TotalMemory]
    call __printhex
    printString .msg_space

    ; Iterate each mode and check its attributes
    mov ecx, 15 ; Get the first 15
    mov eax, [vbeblock + vbe2_infoblock_t.VideoModePtr]

    call __printhex
    printString .msg_newline

    xor esi, esi
    mov si, ax ; si now has the offset
    shr eax, 16; ax now has the segment
    mov gs, ax

.iter_modes:
    xor eax, eax
    mov ax, [gs:si]

    call __printhex
    printString .msg_space

    cmp ax, 0xFFFF
    je .fin

    ; Get Mode information
    push edi
    push ecx
        mov cx, ax; Mode
        mov ax, 0x4F01
        mov di, modeInfo ; Location is ES:DI
        int 0x10
    pop ecx
    pop edi

    ; AL != 0x4F means command is not supported
    cmp al, 0x4F
    jnz .get_modeinfo_failure

    ; Non-zeo AH means there is an error
    cmp ah, 00
    jnz .get_modeinfo_failure

    xor eax, eax
    mov ax, [modeInfo + vbe2_modeinfoblock_t.ModeAttributes]
    call __printhex
    printString .msg_space

    mov eax, [modeInfo + vbe2_modeinfoblock_t.PhysBasePtr]
    call __printhex
    printString .msg_space

    xor eax, eax
    mov ax, [modeInfo + vbe2_modeinfoblock_t.Xresolution]
    call __printhex
    printString .msg_space

    xor eax, eax
    mov ax, [modeInfo + vbe2_modeinfoblock_t.Yresolution]
    call __printhex
    printString .msg_space

    xor eax, eax
    mov al, [modeInfo + vbe2_modeinfoblock_t.BitsPerPixel]
    call __printhex
    printString .msg_space

    xor eax, eax
    mov al, [modeInfo + vbe2_modeinfoblock_t.MemoryModel]
    call __printhex
    printString .msg_space

    xor eax, eax
    mov al, [modeInfo + vbe2_modeinfoblock_t.RedFieldPosition]
    call __printhex
    printString .msg_space

.next_iter_modes:
    add si, 2
    printString .msg_newline
    dec ecx
    jcxz .fin ; Exit loop if asked number items are already printed
    jmp .iter_modes

.get_modeinfo_failure:
    printString .msg_modeget_failed
    jmp .fin
.query_failure:
    printString .msg_modequery_failed
.fin:
    pop gs
    pop es
    popad
    ret

.msg_modequery_failed:  db "0x4F00 failed", 13, 10, 0
.msg_modeget_failed:    db "0x4F01 failed", 13, 10, 0
.msg_newline:           db 13, 10, 0
.msg_space:             db " ", 0

; ---------------------------------------------------------------------------
; Searches for the VBE mode which matches the criterias
; Input:
;   - DS:EDI    :- Pointer to VbeModeQuery structure with properly filled input
;                  fields.
; Output (Success):
;   - DS:EDI    :- Mode number and other output fields are set
;   - Carry     :- Cleared.
; Output (Failue):
;   - Carry     :- Set
; ---------------------------------------------------------------------------
vbe2_find_mode:
    pushad
    push es
    push gs

    push ds     ; Mov es, ds
    pop es

    ; Query the video modes
    push edi
        mov ax, 0x4F00
        mov di, vbeblock
        int 0x10
    pop edi

    ; AL != 0x4F means command is not supported
    cmp al, 0x4F
    jnz .failure

    ; Non-zeo AH means there is an error
    cmp ah, 00
    jnz .failure

    ; Store VBE version
    mov ax, [vbeblock + vbe2_infoblock_t.VbeVersion]
    mov [edi + vbe_modequery_t.VbeVersion], ax

    ; Iterate each mode and check its attributes
    mov eax, [vbeblock + vbe2_infoblock_t.VideoModePtr]

    mov si, ax ; si now has the offset
    shr eax, 16; ax now has the segment
    mov gs, ax

.iter_modes:
    mov ax, [gs:si]
    cmp ax, 0xFFFF
    je .not_found

    ; Store Graphics Mode
    mov [edi + vbe_modequery_t.Mode], ax

    ; Get Mode information
    push edi
        mov cx, ax; Mode
        mov ax, 0x4F01
        mov di, modeInfo
        int 0x10
    pop edi

    ; AL != 0x4F means command is not supported
    cmp al, 0x4F
    jnz .failure

    ; Non-zeo AH means there is an error
    cmp ah, 00
    jnz .failure

    ;-----------------------------------------------------------------
    ; Physical address of frame buffer
    mov eax, [modeInfo + vbe2_modeinfoblock_t.PhysBasePtr]
    cmp eax, 0x0000      ; Mode is not supported
    je .next_iter_modes

    ; Store Physical frame buffer address
    mov [edi + vbe_modequery_t.FrameBuffer], eax

    ;-----------------------------------------------------------------
    ; Store BytesPerScanLine
    xor eax, eax
    mov ax, [modeInfo + vbe2_modeinfoblock_t.BytesPerScanLine]
    mov [edi + vbe_modequery_t.BytesPerScanLine], ax

    ;-----------------------------------------------------------------
    ; Match XResolution
    xor eax, eax
    mov ax, [modeInfo + vbe2_modeinfoblock_t.Xresolution]
    cmp ax, [edi + vbe_modequery_t.Xresolution]
    jne .next_iter_modes

    ;-----------------------------------------------------------------
    ; Match YResolution
    mov ax, [modeInfo + vbe2_modeinfoblock_t.Yresolution]
    cmp ax, [edi + vbe_modequery_t.Yresolution]
    jne .next_iter_modes

    ;-----------------------------------------------------------------
    ; Match BitsPerPixel
    xor eax, eax
    mov al, [modeInfo + vbe2_modeinfoblock_t.BitsPerPixel]
    cmp al, [edi + vbe_modequery_t.BitsPerPixel]
    jne .next_iter_modes

    ;-----------------------------------------------------------------
    ; Must support graphics mode & Linear frame buffer
    xor eax, eax
    mov ax, [modeInfo + vbe2_modeinfoblock_t.ModeAttributes]
    and ax, (1 << 3) | (1 << 7)
    jz .next_iter_modes

    ;-----------------------------------------------------------------
    ; Must be Packed pixel Memory model
    xor eax, eax
    mov al, [modeInfo + vbe2_modeinfoblock_t.MemoryModel]
    cmp al, 0x4    ; Packed Memory model
    je .memorymodel_match_done ; Memory model matched. Match next criteria.

    ; Memory model is not Packed. See if its DirectColor
    cmp al, 0x6    ; DirectColor Memory model
    jne .next_iter_modes ; Did not match

    ; For DirectColor check Red,Green,Blue Mask and FieldPositions
%ifdef GXMODE_BITSPERPIXEL == 32
    xor eax, eax
    ; -- Red
    mov al, [modeInfo + vbe2_modeinfoblock_t.RedMaskSize]
    cmp al, 0x8    ; 8 bits per pixel for 32 bit color mode
    jne .next_iter_modes ; Did not match

    mov al, [modeInfo + vbe2_modeinfoblock_t.RedFieldPosition]
    cmp al, 0x10    ; Of the 4 bytes, Red is byte 3
    jne .next_iter_modes ; Did not match

    ; -- Green
    mov al, [modeInfo + vbe2_modeinfoblock_t.GreenMaskSize]
    cmp al, 0x8    ; 8 bits per pixel for 32 bit color mode
    jne .next_iter_modes ; Did not match

    mov al, [modeInfo + vbe2_modeinfoblock_t.GreenFieldPosition]
    cmp al, 0x8    ; Of the 4 bytes, Green is byte 2
    jne .next_iter_modes ; Did not match

    ; -- Blue
    mov al, [modeInfo + vbe2_modeinfoblock_t.BlueMaskSize]
    cmp al, 0x8    ; 8 bits per pixel for 32 bit color mode
    jne .next_iter_modes ; Did not match

    mov al, [modeInfo + vbe2_modeinfoblock_t.BlueFieldPosition]
    cmp al, 0x0    ; Of the 4 bytes, Blue is byte 1
    jne .next_iter_modes ; Did not match
%endif
.memorymodel_match_done:
    ;-----------------------------------------------------------------

    ; All the criterias match. So we exit
    jmp .found
.next_iter_modes:
    add si, 2
    jmp .iter_modes

.found:
    clc
    jmp .end
.not_found:
.failure:
    stc
.end:
    pop gs
    pop es
    popad
    ret

; ---------------------------------------------------------------------------
; Switches to a VBE mode
; Input:
;   - DS:EDI    :- Pointer to VbeModeQuery structure with properly filled input
;                  fields.
; Output (Success):
;   - Carry     :- Cleared.
; Output (Failue):
;   - Carry     :- Set
; ---------------------------------------------------------------------------
vbe2_switch_mode:
    pushad

    mov bx, [edi + vbe_modequery_t.Mode]
    or  bx, (1 << 14) ; Linear Frame buffer & clear FB memory
    mov ax, 0x4F02
    int 0x10

    ; AL != 0x4F means command is not supported
    cmp al, 0x4F
    jnz .failure

    ; Non-zeo AH means there is an error
    cmp ah, 00
    jnz .failure

    ; Success
    clc
    jmp .end
.failure:
    stc
.end:
    popad
    ret
; ---------------------------------------------------------------------------
; Global Data
; ---------------------------------------------------------------------------
vbeblock:
                db "VBE2"
    times 512-4 db 0

modeInfo:
    times vbe2_modeinfoblock_t_size db 0
; ---------------------------------------------------------------------------
