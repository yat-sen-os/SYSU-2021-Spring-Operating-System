org 0x7e00
[bits 16]
mov ax, 0xb800
mov gs, ax

mov ah, 0x03 ;蓝色
mov al, 'b'
mov [gs:2 * 0], ax

mov al, 'o'
mov [gs:2 * 1], ax

mov al, 'o'
mov [gs:2 * 2], ax

mov al, 't'
mov [gs:2 * 3], ax

mov al, 'l'
mov [gs:2 * 4], ax

mov al, 'o'
mov [gs:2 * 5], ax

mov al, 'a'
mov [gs:2 * 6], ax

mov al, 'd'
mov [gs:2 * 7], ax

mov al, 'e'
mov [gs:2 * 8], ax

mov al, 'r'
mov [gs:2 * 9], ax

jmp $ ; 死循环