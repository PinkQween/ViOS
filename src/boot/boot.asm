ORG 0x7C00
BITS 16

; === Segment Offsets ===
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

jmp short start
nop

; === FAT16 Boot Sector Header ===
OEMIdentifier           db 'VIOS    '
BytesPerSector          dw 0x200
SectorsPerCluster       db 0x80
ReservedSectors         dw 200
FATCopies               db 0x02
RootDirEntries          dw 0x40
NumSectors              dw 0x00
MediaType               db 0xF8
SectorsPerFat           dw 0x100
SectorsPerTrack         dw 0x20
NumberOfHeads           dw 0x40
HiddenSectors           dd 0x00
SectorsBig              dd 0x773594

; === Extended BPB (DOS 4.0+) ===
DriveNumber             db 0x80
WinNTBit                db 0x00
Signature               db 0x29
VolumeID                dd 0xD105
VolumeIDString          db 'VIOS BOOT  '
SystemIDString          db 'FAT16   '

; === Kernel Metadata ===
KernelSizeBytes         dd 0x00000000  ; (Filled by build script)
KernelSizeSectors       dw 0x0000      ; (Filled by build script)

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
    call clear

    ; --- Try VBE Mode 0x17E ---
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
    ; --- Fallback to VBE Mode 0x117 ---
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
    mov eax, 1                              ; LBA start
    movzx ecx, word [KernelSizeSectors]    ; Sector count
    mov edi, 0x0100000                      ; Destination address
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