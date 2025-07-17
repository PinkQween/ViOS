ORG 0x7E00
BITS 16

jmp short load_stage2
nop

; === BIOS Parameter Block ===
OEMIdentifier            db 'MSWIN4.1'
BytesPerSector           dw 512
SectorsPerCluster        db 8
ReservedSectors          dw 32
FATCopies                db 2
RootDirEntries           dw 0
NumSectors               dw 0
MediaType                db 0xF8
SectorsPerFat            dw 0
SectorsPerTrack          dw 63
NumberOfHeads            dw 255
HiddenSectors            dd 2048
SectorsBig               dd 262144

; === FAT32 Extended BPB ===
SectorsPerFat32          dd 2048
ExtFlags                 dw 0
FSVersion                dw 0
RootCluster              dd 2
FSInfoSector             dw 1
BackupBootSector         dw 6
ReservedEx               times 12 db 0
DriveNumber              db 0x80
Reserved1                db 0
BootSignature            db 0x29
VolumeID                 dd 0x12345678
VolumeLabel              db 'VIOS BOOT  '
SystemIDString           db 'FAT32   '

load_stage2:
    ; Debug: Print VBR Entry message
    mov si, vbr_entry_msg
    call print_entry_msg
    
    jmp 0x0000:0x7F00
    
print_entry_msg:
    mov ah, 0x0E
.next_char:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .next_char
.done:
    ret

vbr_entry_msg db "VBR Entry: Loading stage2...", 0

hang:
    cli
    hlt
    jmp hang

times 510-($-$$) db 0
dw 0xAA55
