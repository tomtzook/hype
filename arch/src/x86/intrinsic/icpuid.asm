
bits 64

section .text


global _cpuid
_cpuid:
    push rbx
    push rdx
    mov eax, edi
    mov ecx, esi
    cpuid
    pop rsi
    mov [rsi], eax
    mov [rsi+4], ebx
    mov [rsi+8], ecx
    mov [rsi+0xc], edx
    pop rbx
    ret
