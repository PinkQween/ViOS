[BITS 32]

section .asm

global print:function
global vios_getkey:function
global vios_putchar:function
global vios_malloc:function
global vios_free:function

; void print(const char* filename)
print:
    push ebp
    mov ebp, esp
    push dword[ebp+8]
    mov eax, 1 ; Command print
    int 0x80
    add esp, 4
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

vios_putchar:
    push ebp
    mov ebp, esp
    mov eax, 3 ; Command getkey
    push dword [ebp+8]
    int 0x80
    add esp, 4
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