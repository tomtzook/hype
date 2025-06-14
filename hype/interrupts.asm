bits 64
section .text

extern idt_handler

%macro isr_err_stub 1
isr_stub_%+%1:
    mov rcx, %1         ; int
    mov rdx, [rsp]      ; errorcode
    mov r8, [rsp+8h]    ; rip
    sub rsp, 20h
    call idt_handler
    iretq
%endmacro
%macro isr_no_err_stub 1
isr_stub_%+%1:
    mov rcx, %1     ; int
    mov rdx, 0      ; errorcode
    mov r8, [rsp]   ; rip
    sub rsp, 20h
    call idt_handler
    iretq
%endmacro

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

global isr_stub_table
isr_stub_table:
%assign i 0
%rep    32
    dq isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1
%endrep
