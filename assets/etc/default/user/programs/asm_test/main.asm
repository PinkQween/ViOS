[BITS 32]

section .data
hello_msg db "Hello ASM!", 0  ; null-terminated string

section .text
global _start

_start:
    ; Push the string pointer onto the stack for the system call
    push hello_msg
    mov eax, 5  ; SYSTEM_COMMAND5_PRINT_SERIAL
    int 0x80
    add esp, 4  ; Clean up the stack

    ; Exit syscall afterwards
    xor eax, eax
    xor ebx, ebx      ; exit code 0
    int 0x80