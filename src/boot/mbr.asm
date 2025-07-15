[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00      ; Set stack
    sti

    ; Display MBR start message
    mov si, mbr_msg
    call print_string

    ; Load VBR from sector 2048 (LBA) into 0x0000:0x8000
    ; Using LBA addressing since sector 2048 is beyond CHS limits
    
    ; Set up DAP (Disk Address Packet) for LBA read
    mov si, dap
    mov ah, 0x42        ; Extended read
    mov dl, 0x80        ; Drive 0
    int 0x13
    jc fail

    ; Display success message
    mov si, success_msg
    call print_string

    ; Jump to loaded VBR
    jmp 0x0000:0x8000

fail:
    ; Display error message
    mov si, error_msg
    call print_string
    hlt
    jmp fail

print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

mbr_msg:
    db 'MBR: Starting...', 13, 10, 0
success_msg:
    db 'MBR: VBR loaded OK', 13, 10, 0
error_msg:
    db 'MBR: Failed!', 13, 10, 0

; Disk Address Packet for LBA read
dap:
    db 0x10             ; Size of DAP
    db 0x00             ; Reserved
    dw 0x0001           ; Number of sectors
    dw 0x8000           ; Offset (load VBR here)
    dw 0x0000           ; Segment
    dq 2048             ; LBA of VBR sector

; Pad to beginning of partition table at offset 446
times 446 - ($ - $$) db 0

; === Partition Table ===

; One bootable FAT32 partition starting at LBA 2048
db 0x80                ; bootable
db 0x01, 0x01, 0x00    ; CHS start
db 0x0C                ; FAT32 (LBA)
db 0xFE, 0xFF, 0xFF    ; CHS end
dd 2048                ; LBA start
dd 2048 * 128          ; sectors (4MB, just example)

; Remaining 3 partitions empty
times 3 * 16 db 0

dw 0xAA55