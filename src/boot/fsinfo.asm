; FAT32 FSInfo Sector
; This sector provides information about the filesystem state

ORG 0x0000
BITS 16

; FSInfo sector structure
FSInfo_LeadSig          dd 0x41615252         ; Lead signature "RRaA"
FSInfo_Reserved1        times 480 db 0        ; Reserved bytes
FSInfo_StrucSig         dd 0x61417272         ; Structure signature "rrAa"
FSInfo_Free_Count       dd 0xFFFFFFFF         ; Free cluster count (unknown)
FSInfo_Nxt_Free         dd 0xFFFFFFFF         ; Next free cluster (unknown)
FSInfo_Reserved2        times 12 db 0         ; Reserved bytes
FSInfo_TrailSig         dd 0xAA550000         ; Trail signature

; Pad to 512 bytes
times 512-($-$$) db 0
