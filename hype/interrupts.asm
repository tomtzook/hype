bits 64
section .text

extern idt_handler

%macro isr_err_stub 1
isr_stub_%+%1:
    push rax
    push rcx
    push rdx
    push r8
    push r9
    push r10
    push r11

    mov rcx, %1                 ; int
    mov rdx, [rsp+38h]          ; errorcode
    mov r8, [rsp+40h]           ; rip
    movzx r9, word [rsp+48h]    ; cs

    sub rsp, 20h
    call idt_handler
    add rsp, 20h

    pop r11
    pop r10
    pop r9
    pop r8
    pop rdx
    pop rcx
    pop rax

    add rsp, 8 ; discard error code
    iretq
%endmacro
%macro isr_no_err_stub 1
isr_stub_%+%1:
    mov rcx, %1                 ; int
    mov rdx, 0                  ; errorcode
    mov r8, [rsp+38h]           ; rip
    movzx r9, word [rsp+40h]    ; cs

    sub rsp, 20h
    call idt_handler
    add rsp, 20h

    pop r11
    pop r10
    pop r9
    pop r8
    pop rdx
    pop rcx
    pop rax

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

%assign i 32
%rep    224 ; (256 - 32)
    isr_no_err_stub i
%assign i i+1
%endrep

global isr_stub_table
isr_stub_table:
%assign i 0
%rep    256
    dq isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1
%endrep
