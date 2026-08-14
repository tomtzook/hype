bits 64
section .text

extern idt_handler
extern asm_cpu_store_registers
extern asm_cpu_load_registers


extern isr_report
isr_report:
    ret

%macro isr_err_stub 1
extern isr_stub_%+%1 ; todo: remove
isr_stub_%+%1:
    ; stack state
    ; +48 : registers struct
    ; +40 : ss
    ; +32 : rsp
    ; +24 : rflags
    ; +16 : cs
    ; +08 : rip
    ; +00 : error code

    call isr_report ; todo: remove
    push rcx              ; because we are overriding rcx to load the struct, save it here to be returned to stack later
    lea rcx, [rsp+38h]    ; context registers are at the top of the stack
    sub rsp, 20h
    call asm_cpu_store_registers
    add rsp, 20h

    ; return real rcx
    pop rax
    mov [rcx+10h], rax
    mov rax, rcx

    mov rcx, %1                ; int
    mov rdx, [rsp]             ; errorcode
    mov r8, [rsp+8h]           ; rip
    movzx r9, word [rsp+10h]   ; cs
    mov r10, [rsp+18h]         ; rflags
    mov r11, [rsp+20h]         ; rsp
    movzx r12, word [rsp+28h]  ; ss

    ; fix some registers
    mov [rax+88h], r8   ; rip
    mov [rax+90h], r9w  ; cs
    mov [rax+70h], r10  ; rflags
    mov [rax+80h], r11  ; rsp
    mov [rax+9ah], r12w ; ss

    sub rsp, 20h
    call idt_handler
    add rsp, 20h

    ; todo: consider the error code needing to be popped
    lea rcx, [rsp+30h]    ; context registers are at the top of the stack
    jmp asm_cpu_load_registers ; this will return from the interrupt
%endmacro
%macro isr_no_err_stub 1
extern isr_stub_%+%1 ; todo: remove
isr_stub_%+%1:
    ; stack state
    ; +40 : registers struct
    ; +32 : ss
    ; +24 : rsp
    ; +16 : rflags
    ; +08 : cs
    ; +00 : rip

    call isr_report ; todo: remove
    push rcx              ; because we are overriding rcx to load the struct, save it here to be returned to stack later
    lea rcx, [rsp+30h]    ; context registers are at the top of the stack
    sub rsp, 20h
    call asm_cpu_store_registers
    add rsp, 20h

    ; return real rcx
    pop rax
    mov [rcx+10h], rax
    mov rax, rcx

    mov rcx, %1                ; int
    mov rdx, 0                 ; errorcode
    mov r8, [rsp]              ; rip
    movzx r9, word [rsp+8h]    ; cs
    mov r10, [rsp+10h]         ; rflags
    mov r11, [rsp+18h]         ; rsp
    movzx r12, word [rsp+20h]  ; ss

    ; fix some registers
    mov [rax+88h], r8   ; rip
    mov [rax+90h], r9w  ; cs
    mov [rax+70h], r10  ; rflags
    mov [rax+80h], r11  ; rsp
    mov [rax+9ah], r12w ; ss

    sub rsp, 20h
    call idt_handler
    add rsp, 20h

    lea rcx, [rsp+28h]    ; context registers are at the top of the stack
    jmp asm_cpu_load_registers ; this will return from the interrupt
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
