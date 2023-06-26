test_a20line:
    mov ax, 0x2402
    int 0x15
    ret

disable_a20line:
    mov ax, 0x2400
    int 0x15
    ret

enable_a20line:
    mov ax, 0x2401
    int 0x15
    ret