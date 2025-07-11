[BITS 32]

section .text

global vios_exit
global vios_print
global vios_getkey
global vios_malloc
global vios_free
global vios_putchar
global vios_process_load_start
global vios_system
global vios_process_get_arguments
global vios_sleep
global vios_read
global vios_audio_push
global vios_audio_pop
global vios_audio_control

vios_exit:
    push ebp
    mov ebp, esp
    mov eax, 0
    int 0x80
    pop ebp
    ret

vios_print:
    push ebp
    mov ebp, esp
    push dword [ebp+36]
    push dword [ebp+32]
    push dword [ebp+28]
    push dword [ebp+24]
    push dword [ebp+20]
    push dword [ebp+16]
    push dword [ebp+12]
    mov eax, 1
    int 0x80
    add esp, 28
    pop ebp
    ret

vios_getkey:
    push ebp
    mov ebp, esp
    mov eax, 2
    int 0x80
    pop ebp
    ret

vios_putchar:
    push ebp
    mov ebp, esp
    push dword [ebp+36]
    push dword [ebp+32]
    push dword [ebp+28]
    push dword [ebp+24]
    push dword [ebp+20]
    push dword [ebp+16]
    push dword [ebp+12]
    mov eax, 3
    int 0x80
    add esp, 28
    pop ebp
    ret

vios_malloc:
    push ebp
    mov ebp, esp
    mov eax, 4
    push dword [ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret

vios_free:
    push ebp
    mov ebp, esp
    mov eax, 5
    push dword [ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret

vios_process_load_start:
    push ebp
    mov ebp, esp
    mov eax, 6
    push dword [ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret

vios_system:
    push ebp
    mov ebp, esp
    mov eax, 7
    push dword [ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret

vios_process_get_arguments:
    push ebp
    mov ebp, esp
    mov eax, 8
    push dword [ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret

vios_sleep:
    push ebp
    mov ebp, esp
    mov eax, 9
    push dword [ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret

vios_read:
    push ebp
    mov ebp, esp
    mov eax, 10
    push dword [ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret

vios_audio_push:
    push ebp
    mov ebp, esp
    mov eax, 12
    push dword [ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret

vios_audio_pop:
    push ebp
    mov ebp, esp
    mov eax, 13
    int 0x80
    pop ebp
    ret

vios_audio_control:
    push ebp
    mov ebp, esp
    mov eax, 14
    push dword [ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret