[BITS 32]

section .asm

global _start

_start:
    push message
    mov eax, 1
    int 0x80
    add esp, 4

    jmp $

section .data
message: db 'Hello World', 0
clear: db 32, 0