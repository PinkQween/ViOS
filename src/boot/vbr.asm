ORG 0x7E00
BITS 16

jmp near start
nop

; FAT32 BPB (BIOS Parameter Block)
OEMIdentifier            db 'MSWIN4.1'         ; 0x03 - 8 bytes (standard OEM ID)
BytesPerSector          dw 512                ; 0x0B - bytes per sector
SectorsPerCluster       db 8                  ; 0x0D - sectors per cluster
ReservedSectors         dw 32                 ; 0x0E - reserved sectors
FATCopies               db 2                  ; 0x10 - number of FATs
RootDirEntries          dw 0                  ; 0x11 - root dir entries (0 for FAT32)
NumSectors              dw 0                  ; 0x13 - total sectors (0 for FAT32)
MediaType               db 0xF8               ; 0x15 - media descriptor
SectorsPerFat           dw 0                  ; 0x16 - sectors per FAT (0 for FAT32)
SectorsPerTrack         dw 63                 ; 0x18 - sectors per track
NumberOfHeads           dw 255                ; 0x1A - number of heads
HiddenSectors           dd 2048               ; 0x1C - hidden sectors
SectorsBig              dd 262144             ; 0x20 - large sector count (128MB)

; FAT32 Extended BPB
SectorsPerFat32         dd 2048               ; 0x24 - sectors per FAT32
ExtFlags                dw 0                  ; 0x28 - extension flags
FSVersion               dw 0                  ; 0x2A - filesystem version
RootCluster             dd 2                  ; 0x2C - root directory cluster
FSInfoSector            dw 1                  ; 0x30 - FSInfo sector
BackupBootSector        dw 6                  ; 0x32 - backup boot sector
ReservedEx              times 12 db 0         ; 0x34 - reserved

DriveNumber             db 0x80               ; 0x40 - drive number
Reserved1               db 0                  ; 0x41 - reserved
BootSignature           db 0x29               ; 0x42 - boot signature
VolumeID                dd 0x12345678         ; 0x43 - volume ID
VolumeLabel             db 'VIOS BOOT  '      ; 0x47 - volume label (11 bytes)
SystemIDString          db 'FAT32   '         ; 0x52 - system identifier (8 bytes)

; === Kernel Metadata ===
KernelSizeBytes         dd 117312  ; To be filled by build script
KernelSizeSectors       dw 230      ; To be filled by build script

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

vbe_error_msg db "VBE mode failed", 0

; === Entry Point ===
start:
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
    jmp 0:step2

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

; === Protected Mode Setup ===
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
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x9A
    db 0xCF
    db 0x00

gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92
    db 0xCF
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; === 32-bit Protected Mode Code ===
[BITS 32]
load32:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Enable A20
    in al, 0x92
    or al, 2
    out 0x92, al
    
    ; Move temp kernel sector (VBR data)
    mov esi, 0x90000
    mov edi, 0x20000
    mov ecx, 512
    cld
    rep movsb

    ; Read kernel
    mov eax, 2050                           ; LBA start (absolute - kernel is at LBA 2050)
    movzx ecx, word [KernelSizeSectors]    ; Sector count
    mov edi, 0x0100000                      ; Load to 1 MB
    call ata_lba_read

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