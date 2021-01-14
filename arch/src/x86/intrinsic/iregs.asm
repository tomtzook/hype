
bits 64

section .text

global _read_cr0
_read_cr0:
    mov rax, cr0
    ret

global _write_cr0
_write_cr0:
    mov cr0, rdi
    ret

global _read_cr3
_read_cr3:
    mov rax, cr3
    ret

global _write_cr3
_write_cr3:
    mov cr3, rdi
    ret

global _read_cr4
_read_cr4:
    mov rax, cr4
    ret

global _write_cr4
_write_cr4:
    mov cr4, rdi
    ret
