; VBR (Volume Boot Record) for FAT32 - ViOS
; This VBR is loaded at 0x8000 by the MBR
; It sets up the proper FAT32 filesystem structure

BITS 16

; === Segment Offsets ===
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; --- Standard FAT32 BIOS Parameter Block (BPB) ---
; Jump instruction to skip BPB and start actual boot code
jmp near start
nop

; OEM Name (8 bytes)
OEMName              db 'VIOS    '          ; OEM Name (must be 8 bytes)

; Standard BPB fields
BytesPerSector       dw 512                 ; Bytes per sector
SectorsPerCluster    db 8                   ; Sectors per cluster (4KB clusters)
ReservedSectors      dw 32                  ; Reserved sectors before FAT
NumFATs              db 2                   ; Number of FATs

RootEntryCount       dw 0                   ; 0 for FAT32 (root dir in data area)
TotalSectors16       dw 0                   ; 0 means check TotalSectors32

MediaDescriptor      db 0xF8                ; Fixed disk media descriptor
SectorsPerFAT16      dw 0                   ; 0 for FAT32 (use SectorsPerFAT32)
SectorsPerTrack      dw 63                  ; Typical CHS geometry
NumberOfHeads        dw 255                 ; Typical CHS geometry
HiddenSectors        dd 2048                ; Hidden sectors (LBA start of partition)
TotalSectors32       dd 262144              ; Total sectors in partition (128 MB)

; FAT32 Extended BPB
SectorsPerFAT32      dd 512                 ; Number of sectors per FAT
Flags                dw 0                   ; Flags (bit 7: 0=FAT mirrored, 1=active FAT)
Version              dw 0                   ; Version (0.0)
RootCluster          dd 2                   ; Root directory cluster number
FSInfoSector         dw 1                   ; FSInfo sector number (relative to VBR)
BackupBootSector     dw 6                   ; Backup boot sector number (relative to VBR)
Reserved             db 12 dup (0)          ; Reserved bytes

; Extended boot signature fields
DriveNumber          db 0x80                ; Drive number (0x80 = hard disk)
Reserved1            db 0                   ; Reserved
BootSignature        db 0x29                ; Extended boot signature
VolumeID             dd 0x12345678          ; Volume serial number
VolumeLabel          db 'VIOS FAT32  '      ; Volume label (11 bytes)
FileSystemType       db 'FAT32   '          ; File system type (8 bytes)

; --- ViOS Kernel Metadata (custom extension) ---
KernelSizeBytes      dd 0x00000000          ; Kernel size in bytes (filled by build script)
KernelSizeSectors    dw 0x00F0              ; Kernel size in sectors (240 sectors = 120KB)

; === VBE Fallback Error Display ===
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

vbe_error_msg db "VBE mode failed", 0

; === Boot Entry Point ===
start:
    ; Try VBE Mode 0x117 (1024x768x16)
    mov ax, 0x4F02
    mov bx, 0x117
    int 0x10
    cmp ax, 0x004F
    jne vbe_error

    ; mov si, success_msg ; REMOVED
    ; call serial_print ; REMOVED
    jmp step2

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

; === Protected Mode Transition ===
step2:
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
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:load32

; === GDT Setup ===
gdt_start:
gdt_null:     dq 0

gdt_code:
    dw 0xFFFF           ; Limit
    dw 0x0000           ; Base low
    db 0x00             ; Base mid
    db 0x9A             ; Code segment
    db 11001111b        ; Flags
    db 0x00             ; Base high

gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92             ; Data segment
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; === 32-bit Mode Entry ===
[BITS 32]
load32:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Enable A20 line
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Move temp kernel data
    mov esi, 0x90000
    mov edi, 0x20000
    mov ecx, 512
    cld
    rep movsb

    ; Setup for ATA sector read
    ; Kernel is stored in the FAT32 data area
    ; LBA 2048 = VBR, LBA 3104 = FAT32 data area (kernel start)
    mov eax, 3104                           ; LBA start (FAT32 data area)
    movzx ecx, word [KernelSizeSectors]    ; Sector count from BPB
    mov edi, 0x0100000                      ; Destination address (1MB)
    
    call ata_lba_read
    
    ; Jump to loaded kernel
    jmp CODE_SEG:0x0100000

; === ATA LBA Sector Read ===
ata_lba_read:
    mov ebx, eax

    shr eax, 24
    or eax, 0xE0
    mov dx, 0x1F6
    out dx, al

    mov eax, ecx
    mov dx, 0x1F2
    out dx, al

    mov eax, ebx
    mov dx, 0x1F3
    out dx, al

    mov dx, 0x1F4
    shr eax, 8
    out dx, al

    mov dx, 0x1F5
    shr eax, 8
    out dx, al

    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

.next_sector:
    push ecx
.try_again:
    mov dx, 0x1F7
    in al, dx
    test al, 8
    jz .try_again

    mov ecx, 256
    mov dx, 0x1F0
    rep insw

    pop ecx
    loop .next_sector
    ret

; === Boot Signature ===
times 510-($ - $$) db 0
dw 0xAA55