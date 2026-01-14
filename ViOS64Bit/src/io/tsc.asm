[BITS 64]
section .asm

extern tsc_frequency

global read_tsc
global call_pause
global tsc_microseconds

read_tsc:
    lfence
    rdtsc
    shl rdx, 32
    or  rax, rdx
    ret

call_pause:
    pause
    ret

tsc_microseconds:
    push rbp
    mov rbp, rsp
    sub rsp, 32          ; allocate locals, stack now 16-byte aligned for SysV ABI

    call tsc_frequency
    mov qword [rbp-8], rax  ; store tsc_freq

    call read_tsc
    mov qword [rbp-16], rax ; store tsc_now

    mov rax, qword [rbp-16]
    mov rcx, 1000000
    mul rcx
    mov rcx, qword [rbp-8]
    div rcx                 ; rax = microseconds

    mov rsp, rbp
    pop rbp
    ret