org 0x7c00
bits 16

%define ENDL 0x0d, 0x0a

jmp short start
nop

bdb_oem:
    db 'MSWIN4.1'
bdb_bytes_per_sector:
    dw 512

bdb_sectors_per_cluster:
    db 1

bdb_reserved_sectors:
    dw 1

bdb_fat_count:
    db 2

bdb_dir_entries_count:
    dw 0xe0

bdb_total_sectors:
    dw 2880

bdb_media_descriptor_type:
    db 0xf0

bdb_sectors_per_fat:
    dw 9

bdb_sectors_per_track:
    dw 18

bdb_heads:
    dw 2

bdb_hidden_sectors:
    dd 0

bdb_large_sector_count:
    dd 0

ebr_driver_number:
    db 0
    db 0

ebr_signature:
    db 0x29

ebr_volume_id:
    db 0x12, 0x34, 0x56, 0x78

ebr_volume_label:
    db "Radix_ OS  "

ebr_system_id:
    db 'FAT12   '

start:
    mov ax, 0
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov sp, 0x7c00

    push es
    push word .after
    retf

.after:

    mov [ebr_driver_number], dl

    mov ax, 1
    mov cl, 1
    mov bx, 0x7e00
    call disk_read

    mov si, msg_loading
    call puts

    push es
    mov ah, 0x08
    int 0x13
    jc floppy_error
    pop es

    and cl, 0x3f
    xor cl, cl
    mov [bdb_sectors_per_track], cx

    inc dh
    mov [bdb_heads], dh

    mov ax, [bdb_sectors_per_fat]
    mov bl, [bdb_fat_count]
    xor bh, bh
    mul bx
    add ax, [bdb_reserved_sectors]
    push ax

    mov ax, [bdb_sectors_per_fat]
    shl ax, 5
    xor dx, dx
    div word [bdb_bytes_per_sector]

    test dx, dx
    jz root_dir_after
    inc ax

root_dir_after:

    mov cl, al
    pop ax
    mov dl, [ebr_driver_number]
    mov bx, buffer
    call disk_read

    xor bx, bx
    mov di, buffer

.search_kernel:
    mov si, file_kernel_bin
    mov cx, 11
    push di
    repe cmpsb
    pop di
    je found_kernel
    
    add di, 32
    inc bx
    cmp bx, [bdb_dir_entries_count]
    jl .search_kernel

    jmp kernel_not_found_error

found_kernel:

    cli
    hlt

floppy_error:
    mov si, msg_read_failed
    call puts
    jmp wait_key_and_reboot

kernel_not_found_error:
    mov si, msg_kernel_not_found
    call puts
    jmp wait_key_and_reboot

.halt:
    cli
    hlt

wait_key_and_reboot:
    mov ah, 0
    int 0x16
    jmp 0xffff:0

lba_to_chs:

    push ax
    push dx

    mov dx, 0
    div word [bdb_sectors_per_track]

    inc dx
    mov cx, dx

    mov dx, 0
    div word [bdb_heads]

    mov dh, dl
    mov ch, al
    shl al, 6
    or cl, ah

    pop ax
    mov dl, al
    pop ax
    ret

disk_read:

    push ax
    push bx
    push cx
    push dx
    push di

    push cx
    call lba_to_chs
    pop ax

    mov ah, 0x02
    mov di, 3

    int 0x13

.retry:
    pusha
    stc
    int 0x13
    jnc .done
    popa
    call disk_reset

    dec di
    test di, di
    jnz .retry

.fail:
    jmp floppy_error

.done:
    popa

    pop di
    pop dx
    pop cx
    pop bx
    pop ax
    ret

disk_reset:
    pusha
    mov ah, 0
    stc
    int 0x13
    jc floppy_error
    popa
    ret

msg_loading:
    db "Loading...", ENDL, 0

msg_read_failed:
    db "Read from disk failed!", ENDL, 0

file_kernel_bin:
    db "KERNEL  BIN"

msg_kernel_not_found:
    db "Kernel not found", ENDL, 0

times 510-($-$$) db 0
dw 0xaa55