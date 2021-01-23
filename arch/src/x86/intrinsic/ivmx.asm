
bits 64

section .text


global _vmxon
_vmxon:
    vmxon [rdi]
    jbe _vmx_failure
    jmp _vmx_success

global _vmxoff
_vmxoff:
    vmxoff
    jbe _vmx_failure
    jmp _vmx_success

global _vmclear
_vmclear:
    vmclear [rdi]
    jbe _vmx_failure
    jmp _vmx_success

global _vmread
_vmread:
    vmread [rsi], rdi
    jbe _vmx_failure
    jmp _vmx_success

global _vmwrite
_vmwrite:
    vmwrite rdi, rsi
    jbe _vmx_failure
    jmp _vmx_success

global _vmptrld
_vmptrld:
    vmptrld [rdi]
    jbe _vmx_failure
    jmp _vmx_success

global _vmptrst
_vmptrst:
    vmptrst [rdi]
    jbe _vmx_failure
    jmp _vmx_success

_vmx_failure:
    mov rax, 0x0
    ret

_vmx_success:
    mov rax, 0x1
    ret
