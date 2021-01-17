
bits 64

section .text

global _bit_scan_reverse
_bit_scan_reverse:
    bsr rax, rdi
    ret
