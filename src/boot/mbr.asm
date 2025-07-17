ORG 0x7C00
BITS 16

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax

    mov si, load_msg
    call print

    ; Setup Disk Address Packet (DAP) for LBA 2048
    mov si, dap
    mov ah, 0x42
    mov dl, 0x80        ; First hard disk
    int 0x13
    jc disk_error

    ; Far jump to loaded VBR at 0x0000:0x7E00
    jmp 0x0000:0x7E00

disk_error:
    mov si, error_msg
    call print
    jmp $

print:
    mov ah, 0x0E
.next_char:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .next_char
.done:
    ret

load_msg db "MBR: Loading VBR...", 0
error_msg db "MBR: Disk read error!", 0

; --- Disk Address Packet (DAP) ---
dap:
    db 0x10             ; size of DAP
    db 0x00             ; reserved
    dw 0x0001           ; number of sectors to read
    dw 0x7E00           ; offset
    dw 0x0000           ; segment
    dq 1                ; LBA

times 446 - ($ - $$) db 0  ; Pad to partition table

; === Partition Table ===
db 0x80                  ; Bootable
db 0x01, 0x01, 0x00      ; CHS start — dummy
db 0x0C                  ; FAT32 (LBA)
db 0xFE, 0xFF, 0xFF      ; CHS end — dummy
dd 2048                 ; LBA of first sector of partition
dd 0x773594             ; Number of sectors in partition

times 3*16 db 0          ; Empty partitions
dw 0xAA55               ; Boot signature
