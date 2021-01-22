
bits 64

section .text


global _read_msr
_read_msr:
    mov rcx, rdi
    rdmsr
    shl rdx, 32
    or rax, rdx
    ret

global _write_msr
_write_msr:
    mov rax, rsi
    mov rdx, rsi
    shr rdx, 32
    mov rcx, rdi
    wrmsr
    ret
