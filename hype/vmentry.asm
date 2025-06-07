bits 64
section .text

extern asm_cpu_load_registers

global asm_vm_entry


asm_vm_entry:
    cli
    mov rcx, rsp  ; context registers are at the top of the stack
    sub rsp, 20h
    call asm_cpu_load_registers ; this should return us to last guest rip

    hlt

    ;mov rax, 112h
    ;cmp rax, 0
    ;jne asm_vm_entry

    ;lea rcx, [rsp+10h]  ; context registers are at the top of the stack

    ;sub rsp, 28h
    ;call asm_cpu_load_registers ; this should return us to last guest rip
    ;hlt                         ; should not reach

    ;mov rax, 1
    ;mov rcx, 0
    ;cpuid
