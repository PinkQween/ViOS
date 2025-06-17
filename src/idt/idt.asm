section .asm

extern int21h_handler
extern no_interrupt_handler
extern isr80h_handler

global int21h
global idt_load
global no_interrupt
global enable_interrupts
global disable_interrupts
global isr80h_wrapper

enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret


idt_load:
    push ebp
    mov ebp, esp

    mov ebx, [ebp+8]
    lidt [ebx]
    pop ebp    
    ret


int21h:
    pushad
    call int21h_handler
    popad
    iret

no_interrupt:
    pushad
    call no_interrupt_handler
    popad
    sti
    iret

isr80h_wrapper:
    pushad

    push esp
    
    push eax
    call isr80h_handler
    mov dword[tmp_res], eax
    add esp, 8

    popad
    mov eax, [tmp_res]
    iretd

section .data
tmp_res: dd 0