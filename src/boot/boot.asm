ORG 0x7c00
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

jmp short start
nop

; FAT16 Header
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

; Extended BPB (Dos 4.0)
DriveNumber             db 0x80
WinNTBit                db 0x00
Signature               db 0x29
VolumeID                dd 0xD105
VolumeIDString          db 'VIOS BOOT  '
SystemIDString          db 'FAT16   '


vbe_error:
    mov ax, 0xb800
    mov es, ax
    xor di, di

    mov si, vbe_error_msg

.print_loop:
    lodsb
    cmp al, 0
    je .done
    mov ah, 0x07        ; light gray on black
    mov [es:di], ax
    add di, 2
    jmp .print_loop

.done:
    jmp $

vbe_error_msg db "VBE mode failed", 0

start:
    call clear

    mov ax, 0x4f01        ; querying the VBE
    mov cx, 0x17E         ; Mode we want
    mov bx, 0x090         ; Offset for the vbe info structure
    mov es, bx
    mov di, 0x00
    int 0x10
    cmp ax, 0x004F        ; Check for VBE success
    jne vbe_error         ; Handle error if not successful

    ; Make the switch to graphics mode
    mov ax, 0x4f02
    mov bx, 0x17E
    int 0x10
    cmp ax, 0x004F        ; Check for VBE success
    jne vbe_error         ; Handle error if not successful

    ; Clear segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax

    jmp 0:step2

clear:
    mov ax, 0xb800
    mov es, ax
    xor di, di
    mov cx, 80 * 25     ; total screen chars

.clear_loop:
    mov word [es:di], 0x0720  ; blank char with light gray on black
    add di, 2
    loop .clear_loop
    ret

step2:
    cli                 ; disable interrupts
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    mov sp, 0x7c00      ; stack pointer at 0x7c00
    sti                 ; enable interrupts

.load_protected:
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1           ; enable protected mode bit
    mov cr0, eax
    jmp CODE_SEG:load32

; GDT definitions
gdt_start:
gdt_null:
    dd 0x0
    dd 0x0

; code segment descriptor (offset 0x8)
gdt_code:
    dw 0xffff           ; segment limit low
    dw 0                ; base address low
    db 0                ; base mid
    db 0x9a             ; access byte: present, ring0, code segment, executable, readable
    db 11001111b        ; flags: granularity, 32-bit, limit high
    db 0                ; base high

; data segment descriptor (offset 0x10)
gdt_data:
    dw 0xffff           ; segment limit low
    dw 0                ; base address low
    db 0                ; base mid
    db 0x92             ; access byte: present, ring0, data segment, writable
    db 11001111b        ; flags: granularity, 32-bit, limit high
    db 0                ; base high

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 32]
load32:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Enable A20 line (needed for accessing >1MB memory)
    in al, 0x92
    or al, 2
    out 0x92, al

    mov esi, 0x90000
    mov edi, 0x20000
    mov ecx, 512
    cld
    rep movsb

    ; Prepare for reading sectors via ATA
    mov eax, 1          ; LBA start sector
    mov ecx, 220        ; sector count
    mov edi, 0x0100000  ; destination memory

    call ata_lba_read
    jmp CODE_SEG:0x0100000

ata_lba_read:
    mov ebx, eax        ; Backup the LBA

    ; Send high 8 bits of LBA and select master drive
    shr eax, 24
    or eax, 0xE0
    mov dx, 0x1F6
    out dx, al

    ; Send total sectors to read
    mov eax, ecx
    mov dx, 0x1F2
    out dx, al

    ; Send low 24 bits of LBA in three parts
    mov eax, ebx
    mov dx, 0x1F3
    out dx, al

    mov dx, 0x1F4
    shr eax, 8
    out dx, al

    mov dx, 0x1F5
    shr eax, 16
    out dx, al

    ; Send read command
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

    mov ecx, 256            ; read 256 words (512 bytes)
    mov dx, 0x1F0
    rep insw

    pop ecx
    loop .next_sector

    ret

times 510-($ - $$) db 0
dw 0xAA55
