; Megha Operating System V2 - x86 VESA VBE2.0 driver
; ---------------------------------------------------------------------------

struc vbe2_infoblock_t
    .VbeSignature      : resb 4   ; Vesa Signature
    .VbeVersion        : resw 1   ; VBE Version in BCD
    .OemStringPtr      : resd 1   ; Far pointer to OEM String
    .Capabilities      : resd 1   ; VGA capabilities supported.
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
    pop ebx
    pop eax
    popad
%endmacro

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
        mov di, .vbeblock
        int 0x10
    pop edi

    ; AL != 0x4F means command is not supported
    cmp al, 0x4F
    jnz .failure

    ; Non-zeo AH means there is an error
    cmp ah, 00
    jnz .failure

    ; Store VBE version
    mov ax, [.vbeblock + vbe2_infoblock_t.VbeVersion]
    mov [edi + vbe_modequery_t.VbeVersion], ax

    ; Iterate each mode and check its attributes
    mov eax, [.vbeblock + vbe2_infoblock_t.VideoModePtr]

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
        mov di, .modeInfo
        int 0x10
    pop edi

    ; AL != 0x4F means command is not supported
    cmp al, 0x4F
    jnz .failure

    ; Non-zeo AH means there is an error
    cmp ah, 00
    jnz .failure

    mov eax, [.modeInfo + vbe2_modeinfoblock_t.PhysBasePtr]
    cmp eax, 0x0000      ; Mode is not supported
    je .next_iter_modes

    ; Store Physical frame buffer address
    mov [edi + vbe_modequery_t.FrameBuffer], eax

    ; Store BytesPerScanLine
    xor eax, eax
    mov ax, [.modeInfo + vbe2_modeinfoblock_t.BytesPerScanLine]
    mov [edi + vbe_modequery_t.BytesPerScanLine], ax

    xor eax, eax
    ; Match XResolution
    mov ax, [.modeInfo + vbe2_modeinfoblock_t.Xresolution]
    cmp ax, [edi + vbe_modequery_t.Xresolution]
    jne .next_iter_modes

    ; Match YResolution
    mov ax, [.modeInfo + vbe2_modeinfoblock_t.Yresolution]
    cmp ax, [edi + vbe_modequery_t.Yresolution]
    jne .next_iter_modes

    ; Match BitsPerPixel
    xor eax, eax
    mov al, [.modeInfo + vbe2_modeinfoblock_t.BitsPerPixel]
    cmp al, [edi + vbe_modequery_t.BitsPerPixel]
    jne .next_iter_modes

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
.vbeblock:
    db "VBE2"
    times 512-4 db 0

.modeInfo:
    times vbe2_modeinfoblock_t_size db 0

.blank: db " ", 0
; ---------------------------------------------------------------------------
 
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
