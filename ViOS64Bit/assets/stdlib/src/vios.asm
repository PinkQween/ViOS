[BITS 64]

section .asm

global vios_print:function
global vios_getkey:function
global vios_malloc:function
global vios_free:function
global vios_putchar:function
global vios_process_load_start:function
global vios_process_get_arguments:function
global vios_system:function
global vios_exit:function
global vios_fopen:function
global vios_fclose:function
global vios_fread:function
global vios_fseek:function
global vios_fstat:function
global vios_realloc:function
global vios_fwrite:function

; void vios_print(const char* filename)
vios_print:
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

; int vios_fopen(const char* filename, const char* mode)

vios_fopen:
    mov rax, 10 ; Command 10, fopen
    push qword rsi   ; Pushes the mode 
    push qword rdi   ; Push the filename
    int 0x80        ; call the kernel
    add rsp, 16 ; restore the stack
    ret

; void vios_fclose(size_t fd);
vios_fclose:
    mov rax, 11 ; Command 11 fclose
    push qword rdi 
    add rsp, 8  ; restore the stack
    ret

; long vios_fread(void* buffer, size_t size, size_t count, long fd);
vios_fread:
    mov rax, 12 ; Command 12 fread
    push qword rcx ; fd
    push qword rdx ; count
    push qword rsi ; size
    push qword rdi ; buffer
    int 0x80  ; invoke kernel
    add rsp, 32 ; restore the stack
    ret

; long vios_fseek(long fd, long offset, long whence);
vios_fseek:
    mov rax, 13 ; Command 13 fseek 
    push qword rdx ; whence
    push qword rsi ; offset
    push qword rdi ; fd
    int 0x80       ; invokes the kernel
    add rsp, 24    ; restores the stack
    ret            ; return

; long vios_fstat(long fd, struct file_stat* file_stat_out)
vios_fstat:
    mov rax, 14     ; Command 14 fstat
    push qword rsi  ; file_stat_out
    push qword rdi  ; fd
    int 0x80        ; call kernel
    add rsp, 16     ; restore stack
    ret

; void* vios_realloc(void* old_ptr, size_t new_size);
vios_realloc:
    mov rax, 15     ; Command 15 realloc
    push qword rsi  ; new_size
    push qword rdi  ; old_ptr
    int 0x80
    add rsp, 16
    ; RAX = new the pointer address
    ret

; long vios_fwrite(const void* ptr, size_t size, size_t nmemb, long fd);
vios_fwrite:
    mov rax, 16     ; Command 16 fwrite
    push qword rcx  ; fd
    push qword rdx  ; nmemb
    push qword rsi  ; size
    push qword rdi  ; ptr
    int 0x80        ; invoke kernel
    add rsp, 32     ; restore the stack
    ret