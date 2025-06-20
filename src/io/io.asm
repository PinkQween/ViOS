global insb
global insw
global outb
global outw
global inb

; void insb(uint16_t port);
insb:
    push ebp
    mov ebp, esp

    mov edx, [ebp+8]   ; get port argument (uint16_t)
    xor eax, eax
    in al, dx          ; read byte from port into al

    pop ebp
    ret

; void insw(uint16_t port);
insw:
    push ebp
    mov ebp, esp

    mov edx, [ebp+8]   ; get port argument
    xor eax, eax
    in ax, dx          ; read word from port into ax

    pop ebp
    ret

; void outb(uint16_t port, uint8_t val);
outb:
    push ebp
    mov ebp, esp

    mov edx, [ebp+8]    ; port
    mov eax, [ebp+12]   ; val (byte)
    out dx, al          ; output byte to port

    pop ebp
    ret

; void outw(uint16_t port, uint16_t val);
outw:
    push ebp
    mov ebp, esp

    mov edx, [ebp+8]    ; port
    mov eax, [ebp+12]   ; val (word)
    out dx, ax          ; output word to port

    pop ebp
    ret

; uint8_t inb(uint16_t port);
inb:
    push ebp
    mov ebp, esp

    mov edx, [ebp+8]   ; port
    in al, dx          ; read byte from port
    movzx eax, al      ; zero extend AL to EAX

    pop ebp
    ret