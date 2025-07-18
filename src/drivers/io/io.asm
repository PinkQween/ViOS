global inb
global inw
global inl
global insb
global insw
global outb
global outw
global outl

section .text

; uint8_t inb(uint16_t port);
inb:
    push ebp
    mov ebp, esp

    mov edx, [ebp+8]     ; port
    in al, dx            ; read 8-bit
    movzx eax, al        ; zero-extend to 32-bit

    pop ebp
    ret

; uint16_t inw(uint16_t port);
inw:
    push ebp
    mov ebp, esp

    mov edx, [ebp+8]     ; port
    in ax, dx            ; read 16-bit
    movzx eax, ax        ; zero-extend to 32-bit

    pop ebp
    ret

; uint32_t inl(uint16_t port);
inl:
    push ebp
    mov ebp, esp

    mov edx, [ebp+8]     ; port
    in eax, dx           ; read 32-bit

    pop ebp
    ret

; void insb(uint16_t port, void* buffer, uint32_t count);
insb:
    push ebp
    mov ebp, esp

    mov edx, [ebp+8]     ; port
    mov edi, [ebp+12]    ; buffer
    mov ecx, [ebp+16]    ; count
    cld
    rep insb             ; read ECX bytes from port DX into [EDI]

    pop ebp
    ret

; void insw(uint16_t port, void* buffer, uint32_t count);
insw:
    push ebp
    mov ebp, esp

    mov edx, [ebp+8]     ; port
    mov edi, [ebp+12]    ; buffer
    mov ecx, [ebp+16]    ; count
    cld
    rep insw             ; read ECX words from port DX into [EDI]

    pop ebp
    ret

; void outb(uint16_t port, uint8_t val);
outb:
    push ebp
    mov ebp, esp

    mov edx, [ebp+8]     ; port
    mov eax, [ebp+12]    ; val
    out dx, al           ; write 8-bit

    pop ebp
    ret

; void outw(uint16_t port, uint16_t val);
outw:
    push ebp
    mov ebp, esp

    mov edx, [ebp+8]     ; port
    mov eax, [ebp+12]    ; val
    out dx, ax           ; write 16-bit

    pop ebp
    ret

; void outl(uint16_t port, uint32_t val);
outl:
    push ebp
    mov ebp, esp

    mov edx, [ebp+8]     ; port
    mov eax, [ebp+12]    ; val
    out dx, eax          ; write 32-bit

    pop ebp
    ret