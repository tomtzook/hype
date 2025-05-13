bits 64
section .text

extern asm_cpu_load_registers

global asm_vm_entry


asm_vm_entry:
    lea rcx, [rsp]              ; context registers are at the top of the stack
    call asm_cpu_load_registers ; this should return us to last guest rip
