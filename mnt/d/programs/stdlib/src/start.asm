[BITS 32]

global _start
extern c_start
extern vios_exit

section .asm

_start:
    call c_start
    call vios_exit
    ret