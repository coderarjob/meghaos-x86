

; Sets up the data segments, and jumps to a routine in user mode
; Signature:
; void jump_to_usermode(u32 dataselector, u32 codeselector,
;                       void(*user_func)())
; dataselector : User mode Data segment selector.
; codeselector : User mode Code segment selector.
; stack pointer: User mode stack top virtual address.
; user_func    : Jumps to this function in the user mode code segment.
jump_to_usermode:
    mov ebp, esp
    mov eax, [ebp + 4]          ; Data segment selector.
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push eax                    ; Stack segment selector = DS
    push dword [ebp + 12]       ; User mode stack pointer
    pushfd                      ; EFLAGS
    push dword [ebp + 8]        ; Code Segment selector
    push dword [ebp + 16]       ; Function pointer
    xor ebp, ebp                ; Required for stack trace to work. Ends here.

    iret
global jump_to_usermode

