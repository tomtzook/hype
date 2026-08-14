bits 64
section .text

extern asm_cpu_load_registers
extern on_vmentry_failure
extern on_vmresume_failure

global asm_vm_entry
global asm_vm_resume


asm_vm_entry:
    lea rcx, [rsp]   ; context registers are at the top of the stack
    sub rsp, 20h
    call asm_cpu_load_registers ; this should return us to last guest rip
    add rsp, 20h

    sub rsp, 20h
    call on_vmentry_failure
    add rsp, 20h
    hlt

asm_vm_resume:
    push rax ; todo: remove, temporary
    vmresume

    jc .fail_invalid ; vmfailInvalid (cf=1)
    jz .fail_valid ; vmfailValid (zf=1)
    hlt ; not sure what the hell happened here

.fail_invalid:
    mov eax, 1  ; x86::xmv::instruction_result_t::vm_fail_invalid
    jmp .call_failure
.fail_valid:
    mov eax, 2  ; x86::xmv::instruction_result_t::vm_fail_valid
    jmp .call_failure

.call_failure:
    xor rcx, rcx
    mov ecx, eax
    sub rsp, 20h
    call on_vmresume_failure
    add rsp, 20h
    hlt
