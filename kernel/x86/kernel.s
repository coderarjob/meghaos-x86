bits 32
    
    org 0x0000

_start:
    cli
    xchg bx, bx
    jmp $
