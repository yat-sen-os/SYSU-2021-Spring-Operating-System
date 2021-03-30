[bits 32]

global asm_hello_world
global asm_lidt
global asm_halt
global asm_enable_interrupt
global asm_interrupt_empty_handler
global asm_in_port
global asm_out_port
global asm_time_interrupt_handler
global asm_init_page_reg
global asm_interrupt_status
global asm_disable_interrupt
global asm_switch_thread
extern c_time_interrupt_handler

; void asm_switch_thread(PCB *cur, PCB *next);
asm_switch_thread:
    push ebp
    push ebx
    push edi
    push esi

    mov eax, [esp + 5 * 4]
    mov [eax], esp ; 保存当前栈指针到PCB中，以便日后恢复

    mov eax, [esp + 6 * 4]
    mov esp, [eax] ; 此时栈已经从cur栈切换到next栈

    pop esi
    pop edi
    pop ebx
    pop ebp

    sti
    ret
; int asm_interrupt_status();
asm_interrupt_status:
    xor eax, eax
    pushfd
    pop eax
    and eax, 0x200
    ret

; void asm_disable_interrupt();
asm_disable_interrupt:
    cli
    ret
; void asm_init_page_reg(int *directory);

asm_init_page_reg:
    push ebp
    mov ebp, esp

    push eax

    mov eax, [ebp + 4 * 2]
    mov cr3, eax ; 放入页目录表地址
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax           ; 置PG=1，开启分页机制

    pop eax
    pop ebp

    ret
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