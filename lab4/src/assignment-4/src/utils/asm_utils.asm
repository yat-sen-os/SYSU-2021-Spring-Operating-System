[bits 32]

global asm_hello_world
global asm_lidt
global asm_halt
global asm_enable_interrupt
global asm_interrupt_empty_handler
global asm_in_port
global asm_out_port
global asm_time_interrupt_handler

extern c_time_interrupt_handler

asm_time_interrupt_handler:
    pushad

    nop ; 否则断点打不上去
    ; 发送EOI消息，否则下一次中断不发生
    mov al, 0x20
    out 0x20, al
    out 0xa0, al
    call c_time_interrupt_handler

    popad
    iret
; void asm_out_port(uint16 port, uint8 value)
asm_out_port:
    push ebp
    mov ebp, esp

    push edx
    push eax

    mov edx, [ebp + 4 * 2] ; port
    mov eax, [ebp + 4 * 3] ; value

    out dx, al
    pop eax
    pop edx
    pop ebp
    ret

; void asm_in_port(uint16 port, uint8 *value)
asm_in_port:
    push ebp
    mov ebp, esp

    push edx
    push eax
    push ebx

    xor eax, eax
    mov edx, [ebp + 4 * 2] ; port
    mov ebx, [ebp + 4 * 3] ; *value

    in al, dx
    mov [ebx], al

    pop ebx
    pop eax
    pop edx
    pop ebp
    ret

; void asm_interrupt_empty_handler()
asm_interrupt_empty_handler:
    push 0xdeadbeef
    jmp $
    ret

; void asm_enable_interrupt()
asm_enable_interrupt:
    sti
    ret

; void asm_halt()
asm_halt:
    jmp $

; void asm_lidt(uint32 start, uint16 limit)
asm_lidt:
    push ebp
    mov ebp, esp
    push eax

    mov eax, [ebp + 4 * 3]
    mov [ASM_IDTR], ax
    mov eax, [ebp + 4 * 2]
    mov [ASM_IDTR + 2], eax
    lidt [ASM_IDTR]

    pop eax
    pop ebp
    ret

ASM_IDTR dw 0
         dd 0