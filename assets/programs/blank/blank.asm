[BITS 32]
section .asm

global _start:

_start:
    push 20
    push 30
    mov eax, 0
    int 0x80
    add esp, 8

    jmp $