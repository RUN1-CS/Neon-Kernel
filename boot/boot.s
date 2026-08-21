.intel_syntax noprefix

.code16

.global _start

.text

_start:
    # Execute a far jump to explicitly force CS to 0x07C0
    jmp 0x07C0:sanitize_cs

sanitize_cs:
    mov ax, 0x07C0
    mov ds, ax

    # Store the boot drive number provided by the BIOS in DL
    mov [boot_drive], dl

    # Setup all data and stack segments to 0
    xor ax, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    # Reset the disk controller
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    # Read kernel from disk into RAM address 0x1000
    mov bx, 0x1000
    mov ah, 0x02
    mov al, 15
    mov ch, 0
    mov dh, 0
    mov cl, 2
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    # Transition to 32-bit Protected Mode
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp 0x08:(0x7C00 + init_pm)

.code32
init_pm:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    jmp 0x08:0x1000

disk_error:
    # Write a red 'E' directly to the VGA text mode buffer 
    # to prove the bootloader executed but the disk read failed.
    mov ax, 0xB800
    mov es, ax
    mov word ptr es:[0], 0x4F45
.loop:
    cli
    hlt
    jmp .loop

.section .text

boot_drive: .byte 0

# Global Descriptor Table (GDT)
gdt_start:
    .long 0x0
    .long 0x0
gdt_code:
    .word 0xFFFF
    .word 0x0
    .byte 0x0
    .byte 0x9A
    .byte 0xCF
    .byte 0x0
gdt_data:
    .word 0xFFFF
    .word 0x0
    .byte 0x0
    .byte 0x92
    .byte 0xCF
    .byte 0x0
gdt_end:
gdt_descriptor:
    .word gdt_end - gdt_start - 1
    .long 0x7C00 + gdt_start

.org 510
.word 0xAA55
