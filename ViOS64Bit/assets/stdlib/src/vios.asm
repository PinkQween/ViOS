[BITS 64]

section .asm

global print:function
global vios_getkey:function
global vios_malloc:function
global vios_free:function
global vios_putchar:function
global vios_process_load_start:function
global vios_process_get_arguments:function
global vios_system:function
global vios_exit:function

; void print(const char* filename)
print:
    push rdi
    mov rax, 1 ; Command print
    int 0x80
    add rsp, 8
    ret

; int vios_getkey()
vios_getkey:
    mov rax, 2 ; Command getkey
    int 0x80
    ret

; void vios_putchar(char c)
vios_putchar:
    mov rax, 3 ; Command putchar
    push rdi ; Variable "c"
    int 0x80
    add rsp, 8
    ret

; void* vios_malloc(size_t size)
vios_malloc:
    mov rax, 4 ; Command malloc (Allocates memory for the process)
    push rdi ; Variable "size"
    int 0x80
    add rsp, 8
    ret

; void vios_free(void* ptr)
vios_free:
    mov rax, 5 ; Command 5 free (Frees the allocated memory for this process)
    push rdi ; Variable "ptr"
    int 0x80
    add rsp, 8
    ret

; void vios_process_load_start(const char* filename)
vios_process_load_start:
    mov rax, 6 ; Command 6 process load start ( stars a process )
    push rdi ; Variable "filename"
    int 0x80
    add rsp, 8
    ret

; int vios_system(struct command_argument* arguments)
vios_system:
    mov rax, 7 ; Command 7 process_system ( runs a system command based on the arguments)
    push rdi  ; Variable "arguments"
    int 0x80
    add rsp, 8
    ret


; void vios_process_get_arguments(struct process_arguments* arguments)
vios_process_get_arguments:
    mov rax, 8 ; Command 8 Gets the process arguments
    push rdi ; Variable arguments
    int 0x80
    add rsp, 8
    ret

; void vios_exit()
vios_exit:
    mov rax, 9 ; Command 9 process exit
    int 0x80
    ret
