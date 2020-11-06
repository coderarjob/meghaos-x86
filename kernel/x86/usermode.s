
extern usermode_main
global __jump_to_usermode

__jump_to_usermode:
    mov eax, 0b10_0011          ;  0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push eax
    push esp
    pushf
    push dword 0b1_1011          ;  0x1b
    push usermode_main

    iret

