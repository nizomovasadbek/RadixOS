org 0x0
bits 16

%define ENDL 0x0d, 0x0a

start:
    mov si, msg_hello
    call puts

.halt:
    cli
    hlt

puts:
    push si
    push ax
    push bx

.loop:
    lodsb
    or al, al
    jz .done

    mov ah, 0x0e
    mov bh, 0
    int 0x10

    jmp .loop

.done:
    pop bx
    pop ax
    pop si
    ret

msg_hello:
    db "Radix OS!", ENDL, 0