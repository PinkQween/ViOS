[BITS 32]

section .asm

global vios_exit:function
global vios_print:function
global vios_getkey:function
global vios_malloc:function
global vios_free:function
global vios_putchar:function
global vios_process_load_start:function
global vios_system:function
global vios_process_get_arguments:function
global vios_sleep:function
global vios_read:function
global vios_audio_push:function
global vios_audio_pop:function
global vios_audio_control:function

; void vios_exit()
vios_exit:
    push ebp
    mov ebp, esp
    mov eax, 0 ; Command 0 process exit
    int 0x80
    pop ebp
    ret

; void vios_print(const char* str, int x, int y, int r, int g, int b, int scale)
vios_print:
    push ebp
    mov ebp, esp

    ; Push args in reverse (scale first, str last)
    push dword [ebp+36] ; scale
    push dword [ebp+32] ; b
    push dword [ebp+28] ; g
    push dword [ebp+24] ; r
    push dword [ebp+20] ; y
    push dword [ebp+16] ; x
    push dword [ebp+12] ; str

    mov eax, 1 ; syscall number for print
    int 0x80

    add esp, 28 ; 7 arguments * 4 bytes = 28
    pop ebp
    ret

; int vios_getkey()
vios_getkey:
    push ebp
    mov ebp, esp
    mov eax, 2 ; Command getkey
    int 0x80
    pop ebp
    ret

; void vios_putchar(char str, int x, int y, int r, int g, int b, int scale)
vios_putchar:
    ; Push args in reverse (scale first, str last)
    push dword [ebp+36] ; scale
    push dword [ebp+32] ; b
    push dword [ebp+28] ; g
    push dword [ebp+24] ; r
    push dword [ebp+20] ; y
    push dword [ebp+16] ; x
    push dword [ebp+12] ; c

    mov eax, 3 ; syscall number for putchar
    int 0x80

    add esp, 28 ; 7 arguments * 4 bytes = 28
    pop ebp
    ret

; void* vios_malloc(size_t size)
vios_malloc:
    push ebp
    mov ebp, esp
    mov eax, 4 ; Command malloc (Allocates memory for the process)
    push dword[ebp+8] ; Variable "size"
    int 0x80
    add esp, 4
    pop ebp
    ret

; void vios_free(void* ptr)
vios_free:
    push ebp
    mov ebp, esp
    mov eax, 5 ; Command 5 free (Frees the allocated memory for this process)
    push dword[ebp+8] ; Variable "ptr"
    int 0x80
    add esp, 4
    pop ebp
    ret

; void vios_process_load_start(const char* filename)
vios_process_load_start:
    push ebp
    mov ebp, esp
    mov eax, 6 ; Command 6 process load start ( stars a process )
    push dword[ebp+8] ; Variable "filename"
    int 0x80
    add esp, 4
    pop ebp
    ret

; int vios_system(struct command_argument* arguments)
vios_system:
    push ebp
    mov ebp, esp
    mov eax, 7 ; Command 7 process_system ( runs a system command based on the arguments)
    push dword[ebp+8] ; Variable "arguments"
    int 0x80
    add esp, 4
    pop ebp
    ret


; void vios_process_get_arguments(struct process_arguments* arguments)
vios_process_get_arguments:
    push ebp
    mov ebp, esp
    mov eax, 8 ; Command 8 Gets the process arguments
    push dword[ebp+8] ; Variable arguments
    int 0x80
    add esp, 4
    pop ebp
    ret

; void vios_sleep(int seconds)
vios_sleep:
    push ebp
    mov ebp, esp
    mov eax, 9
    push dword[ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret

; char* vios_read(const char* filename)
vios_read:
    push ebp
    mov ebp, esp
    mov eax, 10
    push dword[ebp+8]
    int 0x80
    add esp, 4
    pop ebp
    ret

; void vios_audio_push(char c)
vios_audio_push:
    push ebp
    mov ebp, esp
    mov eax, 12 ; Command 12 audio_push
    push dword[ebp+8] ; Variable "c"
    int 0x80
    add esp, 4
    pop ebp
    ret

; char vios_audio_pop()
vios_audio_pop:
    push ebp
    mov ebp, esp
    mov eax, 13 ; Command 13 audio_pop
    int 0x80
    pop ebp
    ret

; void vios_audio_control(int command)
vios_audio_control:
    push ebp
    mov ebp, esp
    mov eax, 14 ; Command 14 audio_control
    push dword[ebp+8] ; Variable "command"
    int 0x80
    add esp, 4
    pop ebp
    ret
