; MBR FAT32 Bootloader - loads first cluster of BOOT    file
BITS 16
ORG 0x7C00

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Save boot drive
    mov [boot_drive], dl

    ; Set text mode
    mov ax, 0x03
    int 0x10

    ; Load first sector of partition at LBA 2048
    mov eax, 2048               ; Sector number to load
    mov [dap_lba], eax
    mov [dap_mem], word 0x8000  ; Destination segment:offset
    mov [dap_mem + 2], word 0x0000

    ; INT 13h Extended Read (0x42)
    mov si, disk_address_packet
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc disk_error

    ; Jump to loaded VBR/Stage2
    jmp 0:0x8000

disk_error:
    mov si, err_msg
.print_err:
    lodsb
    or al, al
    jz $
    mov ah, 0x0E
    int 0x10
    jmp .print_err

; --- Disk Address Packet ---
disk_address_packet:
    db 0x10       ; Size
    db 0x00       ; Reserved
    dw 1          ; Sectors to read
dap_mem:
    dw 0x8000     ; Offset
    dw 0x0000     ; Segment
dap_lba:
    dd 0x00000800 ; LBA = 2048
    dd 0x00000000 ; LBA high dword

boot_drive: db 0
err_msg db "Disk Error", 0

times 510 - ($ - $$) db 0
dw 0xAA55