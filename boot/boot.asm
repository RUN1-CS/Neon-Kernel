BITS 16
ORG 0x7C00

start:
    ; Execute a far jump to explicitly force CS to 0x0000
    jmp 0x0000:sanitize_cs

sanitize_cs:
    ; Store the boot drive number provided by the BIOS in DL
    mov [boot_drive], dl

    ; Setup all data and stack segments to 0
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Reset the disk controller
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    ; Read kernel from disk into RAM address 0x1000
    mov bx, 0x1000       ; Destination memory offset
    mov ah, 0x02         ; BIOS read sectors command
    mov al, 15           ; Number of sectors to read
    mov ch, 0            ; Cylinder 0
    mov dh, 0            ; Head 0
    mov cl, 2            ; Sector 2 (Kernel start)
    mov dl, [boot_drive] ; Boot drive target
    int 0x13
    jc disk_error

    ; Transition to 32-bit Protected Mode
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or eax, 0x1          ; Set PE bit
    mov cr0, eax

    jmp 0x08:init_pm     ; Far jump to clear the pipeline

BITS 32
init_pm:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    jmp 0x1000           ; Direct jump into your kernel_main()

disk_error:
    ; Write a red 'E' directly to the VGA text mode buffer 
    ; to prove the bootloader executed but the disk read failed.
    mov word [0xB8000], 0x4F45
.loop:
    cli
    hlt
    jmp .loop

boot_drive: db 0

; Global Descriptor Table (GDT)
gdt_start:
    dd 0x0
    dd 0x0
gdt_code:
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0
gdt_data:
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510-($-$$) db 0
dw 0xAA55