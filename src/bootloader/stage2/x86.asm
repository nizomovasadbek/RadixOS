bits 16

section _TEXT class=CODE

global _x86_div64_32
_x86_div64_32:

    push bp
    mov bp, sp
    push bx

    mov eax, [bp + 8]
    mov ecx, [bp + 12]
    mov edx, 0
    div ecx

    mov ebx, [bp + 16]
    mov [bx + 4],  eax

    mov eax, [bp + 4]
    div ecx

    mov [bx], eax
    mov bx, [bp + 18]
    mov [bx], edx

    pop bx
    mov sp, bp
    pop bp
    ret

global _x86_VideoWriteChatTeletype
_x86_VideoWriteChatTeletype:
    push bp
    mov bp, sp

    push bx

    mov ah, 0x0E
    mov al, [bp + 4]
    mov bh, [bp + 6]

    int 0x10

    pop bx
    mov sp, bp

    pop bp
    ret

global _x86_Disk_Reset
_x86_Disk_Reset:

    push bp
    mov bp, sp

    mov ah, 0
    mov dl, [bp+4]
    stc
    int 0x13

    mov ax, 1
    sbb ax, 0

    mov sp, bp
    pop bp
    ret

global _x86_Disk_Read
_x86_Disk_Read:

    push bp
    mov bp, sp

    push bx
    push es

    mov dl, [bp + 4]
    mov ch, [bp + 6]
    mov cl, [bp + 7]
    shl cl, 6

    mov dh, [bp + 8]
    mov al, [bp + 10]
    and al, 0x3F
    or cl, al

    mov al, [bp + 12]

    mov bx, [bp + 16]
    mov es, bx
    mov bx, [bp + 14]
    mov ah, 0x02

    stc
    int 0x13

    mov ax, 1
    sbb ax, 0

    pop es
    pop bx

    mov sp, bp
    pop bp
    ret