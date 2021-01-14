
bits 64

section .text

global _sgdt
_sgdt:
    sgdt [rdi]
    ret

global _lgdt
_lgdt:
    lgdt [rdi]
    ret
