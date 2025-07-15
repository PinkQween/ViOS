[BITS 16]
[ORG 0x7C00]

jmp short start
nop

; === GDT ===
gdt_start:
gdt_null:   dq 0
gdt_code:   dw 0xFFFF, 0x0000, 0x9A00, 0x00CF
gdt_data:   dw 0xFFFF, 0x0000, 0x9200, 0x00CF
gdt_end:

gdtr:       dw 0
            dd 0

CODE_SEG equ 0x08
DATA_SEG equ 0x10

; --- BIOS Parameter Block (FAT32) ---
OEMLabel:           db 'VIOS    '
BytesPerSector:     dw 512
SectorsPerCluster:  db 8
ReservedSectors:    dw 32
NumberOfFATs:       db 2
RootEntries:        dw 0
TotalSectors16:     dw 0
MediaDescriptor:    db 0xF8
SectorsPerFAT16:    dw 0
SectorsPerTrack:    dw 63
NumberOfHeads:      dw 255
HiddenSectors:      dd 2048
TotalSectors32:     dd 274432

SectorsPerFAT32:    dd 256
ExtFlags:           dw 0
FSVersion:          dw 0
RootCluster:        dd 2
FSInfo:             dw 1
BackupBootSector:   dw 6
Reserved:           db 12 dup(0)

DriveNumber:        db 0x80
Reserved1:          db 0
BootSignature:      db 0x29
VolumeID:           dd 0x12345678
VolumeLabel:        db 'VIOS BOOT  '
FileSystemType:     db 'FAT32   '

; === Kernel Information ===
boot_drive:             db 0
kernel_start_sector:    dd 2049
kernel_size_sectors:    dw 4 ; patch as needed

; === Code begins ===
start:
    lea si, msg_start
    call serial_send_string
    ; Save drive
    mov [boot_drive], dl

    ; Setup stack
    xor ax, ax
    mov ss, ax
    mov sp, 0x7C00

    call init_serial
    call serial_send_R

    lea si, msg_before_kernel
    call serial_send_string
    call load_kernel
    lea si, msg_after_kernel
    call serial_send_string

    call serial_send_L

    lea si, msg_before_prot
    call serial_send_string
    call enter_protected

; === Serial routines ===
init_serial:
    mov dx, 0x3FB
    mov al, 0x80
    out dx, al
    mov dx, 0x3F8
    mov al, 0x03
    out dx, al
    mov dx, 0x3F9
    mov al, 0x00
    out dx, al
    mov dx, 0x3FB
    mov al, 0x03
    out dx, al
    ret

serial_wait:
    mov dx, 0x3FD
    in al, dx
    test al, 0x20
    jz serial_wait
    ret

serial_send:
    call serial_wait
    mov dx, 0x3F8
    out dx, al
    ret

serial_send_R:
    mov al, 'R'
    call serial_send
    ret

serial_send_L:
    mov al, 'L'
    call serial_send
    ret

serial_send_G:
    mov al, 'G'
    call serial_send
    ret

serial_send_P:
    mov al, 'P'
    call serial_send
    ret

serial_send_J:
    mov al, 'J'
    call serial_send
    ret

serial_send_string:
    pusha
.next_char:
    lodsb
    or al, al
    jz .done
    call serial_send
    jmp .next_char
.done:
    popa
    ret

; === Kernel loading with INT 13h Extensions ===
load_kernel:
    mov edi, 0x100000
    mov eax, [kernel_start_sector]
    mov cx, [kernel_size_sectors]

.read:
    pusha
    mov si, dap
    mov dl, [boot_drive]
    mov [dap+4], di
    mov [dap+6], word 0
    mov [dap+8], eax
    mov [dap+2], word 1
    mov ah, 0x42
    int 0x13
    popa
    add edi, 512
    inc eax
    dec cx
    jnz .read
    ret

; === Enter protected mode ===
enter_protected:
    cli

    ; Copy GDT to known address (0x0500)
    lea si, [gdt_start]
    mov di, 0x0500
    mov cx, gdt_end - gdt_start
    rep movsb

    ; Setup descriptor
    mov word [gdtr], gdt_end - gdt_start - 1
    mov dword [gdtr + 2], 0x00000500

    lgdt [gdtr]

    call serial_send_G

    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_entry ; far jump

; === 32-bit code starts here ===
[bits 32]
protected_entry:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x200000

    call serial_send_P

    ; Jump to kernel
    call serial_send_J
    jmp CODE_SEG:0x100000

; === DAP ===
[bits 16]
dap:
    db 0x10, 0
    dw 1
    dw 0, 0
    dd 0, 0

; Pad to 510 bytes
times 510-($-$$) db 0
dw 0xAA55

msg_start: db 'VBR: start', 0
msg_before_kernel: db 'VBR: before kernel', 0
msg_after_kernel: db 'VBR: after kernel', 0
msg_before_prot: db 'VBR: before prot', 0
