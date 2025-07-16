[BITS 32]

section .text.start

global _start
global kernel_registers
extern kernel_main

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp

    ; Clear screen to red color for debugging - if we see red, we reached kernel
    mov edi, 0xb8000  ; VGA text buffer
    mov ecx, 80 * 25  ; 80 columns * 25 rows
    mov ax, 0x4f20    ; Red background (0x4f) + space character (0x20)
    rep stosw         ; Fill screen with red spaces

    ; Remap the master PIC
    mov al, 00010001b
    out 0x20, al ; Tell master PIC

    mov al, 0x20 ; Interrupt 0x20 is where master ISR should start
    out 0x21, al
    
    mov al, 0x04 ; ICW3
    out 0x21, al

    mov al, 00000001b
    out 0x21, al
    ; End remap of the master PIC

    ; Remap the slave PIC
    mov al, 00010001b
    out 0xA0, al ; Tell slave PIC

    mov al, 0x28 ; Interrupt 0x28 is where slave ISR should start
    out 0xA1, al
    
    mov al, 0x02 ; ICW3 - tell slave its cascade identity
    out 0xA1, al

    mov al, 00000001b
    out 0xA1, al
    ; End remap of the slave PIC

    ; Keep all IRQs masked initially (will be enabled selectively)
    mov al, 0xFF
    out 0x21, al ; Master PIC
    out 0xA1, al ; Slave PIC

    call kernel_main

    jmp $

kernel_registers:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    ret

times 512-($ - $$) db 0