global jump_to_userprocess
global jump_to_kernelprocess

;-------------------------------------------------------------------------------
; Sets up the stack pointer, and jumps to a routine in kernel mode
; Signature:
; void jump_to_kernelprocess (void* stackTop, void (*user_func)())
;
; stack pointer: User mode stack top virtual address.
; user_func    : Jumps to this function in the user mode code segment.
jump_to_kernelprocess:
    mov ebp, esp
    mov eax, [ebp + 4]       ; Stack pointer
    mov ebx, [ebp + 8]       ; Function pointer
    xor ebp, ebp             ; Required for stack trace to work. Ends here.
    mov esp, eax
    jmp ebx

;-------------------------------------------------------------------------------
; Sets up the data segments, stack pointer and jumps to a routine in user mode
; Signature:
; void jump_to_userprocess (U32 dataselector, U32 codeselector, 
;                        void* stackTop, void (*user_func)())
;
; dataselector : User mode Data segment selector.
; codeselector : User mode Code segment selector.
; stack pointer: User mode stack top virtual address.
; user_func    : Jumps to this function in the user mode code segment.
jump_to_userprocess:
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
;-------------------------------------------------------------------------------
