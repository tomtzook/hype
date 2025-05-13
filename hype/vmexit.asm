bits 64
section .text

extern asm_cpu_store_registers
extern vm_exit_handler

global asm_vm_exit


asm_vm_exit:
    push rcx                     ; because we are overriding rcx to load the struct, save it here to be returned to stack later
    lea rcx, [rsp+8h]            ; context registers are at the top of the stack
    call asm_cpu_store_registers

    pop rax                      ; return real rcx
    mov [rcx+10h], rax

    jmp vm_exit_handler
