
bits 64

section .text


global _read_msr
_read_msr:
    mov ecx, edi
    rdmsr
    shl rdx, 32
    or rax, rdx
    ret

global _write_msr
_write_msr:
    shr rsi, 32
    mov edx, esi
    wrmsr
    ret
