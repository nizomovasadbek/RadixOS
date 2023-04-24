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

    mov dh, [bp + 10]
    mov al, [bp + 8]
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

global _x86_Disk_GetDriveParams
_x86_Disk_GetDriveParams:

    push bp
    mov bp, sp

    push bx
    push si
    push es
    push di

    mov dl, [bp + 4]
    mov ax, 0x08
    mov di, 0
    mov es, di
    stc
    int 0x13

    mov ax, 1
    sbb ax, 0

    mov si, [bp + 6]
    mov [si], bl

    mov bl, ch
    mov bh, cl
    shr bh, 6
    mov si, [bp + 8]
    mov [si], bx

    xor ch, ch
    add cl, 0x3F
    mov si, [bp + 10]
    mov [si], cx

    mov cl, dh
    mov si, [bp + 12]
    mov [si], cx

    pop di
    pop es
    pop si
    pop bx

    mov sp, bp
    pop bp