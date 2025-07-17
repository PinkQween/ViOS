ORG 0x7F00
BITS 16

jmp start

; === Custom kernel info ===
KernelStartSector        dd 2070
KernelSizeSectors        dw 230

vbe_error_msg db "VBE mode failed", 0
msg_hello db 'BOOTLOADER OK', 13, 10, 0

init_serial:
    mov dx, 0x3F8      ; COM1 base port
    mov al, 0x00       ; Disable interrupts
    out dx, al

    mov dx, 0x3FB      ; Line Control Register (LCR)
    mov al, 0x80       ; Enable DLAB (set baud rate divisor)
    out dx, al

    mov dx, 0x3F8      ; Set divisor to 3 (38400 baud)
    mov al, 0x03
    out dx, al
    mov dx, 0x3F9
    mov al, 0x00
    out dx, al

    mov dx, 0x3FB      ; 8 bits, no parity, one stop bit
    mov al, 0x03
    out dx, al

    mov dx, 0x3FC      ; FIFO control register
    mov al, 0xC7       ; Enable FIFO, clear them, 14-byte threshold
    out dx, al

    mov dx, 0x3FE      ; Modem control
    mov al, 0x0B       ; IRQs enabled, RTS/DSR set
    out dx, al
    ret

serial_write_char:
    mov dx, 0x3FD        ; LSR
.wait:
    in al, dx
    test al, 0x20        ; Bit 5 = Transmit Holding Register Empty (THRE)
    jz .wait
    mov dx, 0x3F8        ; Transmit buffer
    mov al, bl
    out dx, al
    ret

serial_print:
    pusha
.next_char:
    lodsb
    test al, al
    jz .done
    mov bl, al
    call serial_write_char
    jmp .next_char
.done:
    popa
    ret

; === VBE Error Message ===
vbe_error:
    mov ax, 0xB800
    mov es, ax
    xor di, di
    mov si, vbe_error_msg
.print_loop:
    lodsb
    cmp al, 0
    je .done
    mov ah, 0x07
    mov [es:di], ax
    add di, 2
    jmp .print_loop
.done:
    jmp $

; === Entry Point ===
start:
    call init_serial

    ; Try VBE Mode 0x17E
    mov ax, 0x4F01
    mov cx, 0x17E
    mov bx, 0x090
    mov es, bx
    xor di, di
    int 0x10
    cmp ax, 0x004F
    jne try_117_query

    mov ax, 0x4F02
    mov bx, 0x17E
    int 0x10
    cmp ax, 0x004F
    jne try_117_query

    jmp vbe_success

try_117_query:
    ; Fallback to VBE Mode 0x117
    mov ax, 0x4F01
    mov cx, 0x117
    mov bx, 0x090
    mov es, bx
    xor di, di
    int 0x10
    cmp ax, 0x004F
    jne vbe_error

    mov ax, 0x4F02
    mov bx, 0x117
    int 0x10
    cmp ax, 0x004F
    jne vbe_error

vbe_success:
    call clear

    xor ax, ax
    mov ds, ax
    mov es, ax
    jmp step2

; === Print Message Function ===
print_msg:
    mov ah, 0x0E
.next_char:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .next_char
.done:
    ret

; === Clear Screen ===
clear:
    mov ax, 0xB800
    mov es, ax
    xor di, di
    mov cx, 80 * 25
.clear_loop:
    mov word [es:di], 0x0720
    add di, 2
    loop .clear_loop
    ret

; === Enter Protected Mode ===
step2:
    ; Debug: Send VBE success marker
    mov bl, 'O'
    call serial_write_char
    mov bl, 'K'
    call serial_write_char
    mov bl, 10
    call serial_write_char

    ; === Enable protected mode ===
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    mov sp, 0x7C00
    sti

.load_protected:
    cli
    
    ; Debug: Send GDT loading marker
    mov bl, 'G'
    call serial_write_char
    mov bl, 'D'
    call serial_write_char
    mov bl, 'T'
    call serial_write_char
    mov bl, 10
    call serial_write_char
    
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Debug: Send marker before jump
    mov bl, 'J'
    call serial_write_char
    mov bl, 'M'
    call serial_write_char
    mov bl, 'P'
    call serial_write_char
    mov bl, 10
    call serial_write_char
    
    ; Manual far jump with inline target
    db 0xEA              ; Far jump opcode
    dw protected_mode_start ; Jump to code right after this
    dw 0x08              ; Code segment (CODE_SEG = 8)
    
; === Protected Mode Entry Point ===
[BITS 32]
protected_mode_start:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; Debug: Send protected mode success marker
    mov bl, '3'
    call serial_write_char_32
    mov bl, '2'
    call serial_write_char_32
    mov bl, 10
    call serial_write_char_32
    
    ; Enable A20
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Read kernel sectors from disk
    mov eax, KernelStartSector           ; LBA start
    movzx ecx, word [KernelSizeSectors]
    mov edi, 0x100000       ; Kernel load address (1 MB)
    call ata_lba_read

    ; Jump to kernel
    jmp 0x8:0x100000

; === GDT ===
gdt_start:
gdt_null:    
    dq 0x0

gdt_code:     
    dw 0xFFFF            ; Limit low
    dw 0x0000            ; Base low
    db 0x00              ; Base middle
    db 0x9A              ; Access
    db 0xCF              ; Flags + Limit high nibble
    db 0x00              ; Base high

gdt_data:     
    dw 0xFFFF            ; Limit low
    dw 0x0000            ; Base low
    db 0x00              ; Base middle
    db 0x92              ; Access (read/write, accessed = 0)
    db 0xCF              ; Flags + Limit high nibble
    db 0x00              ; Base high

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; === Protected Mode (32-bit) ===
[BITS 32]
serial_write_char_32:
    mov dx, 0x3FD        ; LSR
.wait:
    in al, dx
    test al, 0x20        ; Bit 5 = Transmit Holding Register Empty (THRE)
    jz .wait
    mov dx, 0x3F8        ; Transmit buffer
    mov al, bl
    out dx, al
    ret

load32_real:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; Debug: Send protected mode success marker
    mov bl, '3'
    call serial_write_char_32
    mov bl, '2'
    call serial_write_char_32
    mov bl, 10
    call serial_write_char_32
    
    ; Enable A20
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Read kernel sectors from disk
    mov eax, KernelStartSector           ; LBA start
    movzx ecx, word [KernelSizeSectors]
    mov edi, 0x100000       ; Kernel load address (1 MB)
    call ata_lba_read

    ; Jump to kernel
    jmp 0x8:0x100000

; === ATA LBA Sector Read ===
ata_lba_read:
    pushad
    mov esi, ecx            ; Save sector count in ESI
    
.next_sector:
    mov ebx, eax            ; Save LBA
    
    ; Select drive and set LBA mode
    mov al, bl
    shr eax, 24
    and eax, 0x0F
    or eax, 0xE0            ; LBA mode, master drive
    mov dx, 0x1F6
    out dx, al

    ; Set sector count
    mov al, 1               ; Read 1 sector at a time
    mov dx, 0x1F2
    out dx, al

    ; Set LBA address
    mov eax, ebx
    mov dx, 0x1F3
    out dx, al              ; LBA[7:0]

    mov dx, 0x1F4
    shr eax, 8
    out dx, al              ; LBA[15:8]

    mov dx, 0x1F5
    shr eax, 8
    out dx, al              ; LBA[23:16]

    ; Issue READ command
    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

    ; Wait for BSY to clear and DRQ to set
.wait_ready:
    mov dx, 0x1F7
.wait:
    in al, dx
    test al, 0x80           ; BSY bit
    jnz .wait               ; Wait while busy
    test al, 0x08           ; DRQ bit
    jz .wait                ; Wait for data ready
    test al, 0x01           ; ERR bit
    jnz .disk_error         ; Check for error

.read_data:
    mov ecx, 256            ; 256 words = 512 bytes
    mov dx, 0x1F0
    rep insw

    ; Move to next sector
    add edi, 512
    inc ebx
    mov eax, ebx
    dec esi
    jnz .next_sector

    popad
    ret
    
.disk_error:
    ; Send error message over serial
    mov bl, 'E'
    call serial_write_char_32
    mov bl, 'R'
    call serial_write_char_32
    mov bl, 'R'
    call serial_write_char_32
    mov bl, 10
    call serial_write_char_32
    jmp $                   ; Hang
