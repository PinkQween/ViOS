[BITS 64]
section .asm

global cpuid

cpuid:
    push rbx

    mov eax, edi
    mov ecx, esi
    cpuid

    mov [rdx], eax
    mov [rcx], ebx
    mov [r8],  ecx
    mov [r9],  edx

    pop rbx
    ret