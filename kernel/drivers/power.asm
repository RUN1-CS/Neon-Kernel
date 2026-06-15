BITS 32
global shutdown

shutdown:
    mov ax, 0x5301      ; APM: Connect real mode interface
    xor bx, bx          ; Device ID: 0000h (System BIOS)
    int 0x15
    jc .failed

    mov ax, 0x530e      ; APM: Driver version
    xor bx, bx          ; Device ID: 0000h (System BIOS)
    mov cx, 0x0102      ; Version 1.2
    int 0x15
    jc .failed

    mov ax, 0x5307      ; APM: Set power state
    mov bx, 0x0001      ; Device ID: 0001h (All devices managed by APM)
    mov cx, 0x0003      ; State: 0003h (Off)
    int 0x15

.failed:
    cli
    hlt
    jmp .failed