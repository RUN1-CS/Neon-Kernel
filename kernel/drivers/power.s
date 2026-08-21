.intel_syntax noprefix

.global shutdown

.code32

shutdown:
    mov ax, 0x5301
    xor bx, bx
    int 0x15
    jc .failed

    mov ax, 0x530e
    xor bx, bx
    mov cx, 0x0102
    int 0x15
    jc .failed

    mov ax, 0x5307
    mov bx, 0x0001
    mov cx, 0x0003
    int 0x15

.failed:
    cli
    hlt
    jmp .failed
