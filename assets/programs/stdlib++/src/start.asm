[BITS 32]

global _start
extern cpp_start
extern vios_exit

section .asm

_start:
    call cpp_start
    call vios_exit
    ret