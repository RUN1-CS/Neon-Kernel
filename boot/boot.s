.intel_syntax noprefix

.code16

.global _start

# Number of 512-byte sectors to load, starting at disk sector 1 (the sector
# after this one). The Makefile passes the real kernel size:
#   as --32 --defsym KERNEL_SECTORS=<n> boot.s
.ifndef KERNEL_SECTORS
.set KERNEL_SECTORS, 64
.endif

.set KERNEL_SEG, 0x1000          # kernel is loaded at physical 0x10000

.text

_start:
    # Execute a far jump to explicitly force CS to 0x07C0
    jmp 0x07C0:sanitize_cs

sanitize_cs:
    mov ax, 0x07C0
    mov ds, ax

    # Store the boot drive number provided by the BIOS in DL
    mov [boot_drive], dl

    # Setup stack segment
    xor ax, ax
    mov ss, ax
    mov sp, 0x7C00

    # Reset the disk controller
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    # Ask the BIOS for this drive's geometry. A floppy reports 18 sectors x
    # 2 heads, a hard disk (e.g. QEMU's default IDE disk) reports 63 x 16, so
    # the LBA->CHS conversion below must not hard-code either.
    xor ax, ax
    mov es, ax
    xor di, di                   # ES:DI = 0000:0000 (some BIOSes need this)
    mov ah, 0x08
    mov dl, [boot_drive]
    int 0x13
    jc disk_error
    mov al, cl
    and ax, 0x003F               # AX = sectors per track (CL bits 0-5)
    jz disk_error
    mov [spt], ax
    mov al, dh
    xor ah, ah
    inc ax                       # AX = number of heads (DH = highest head)
    mov [heads], ax

    # Load the kernel one sector at a time. One sector per call can never
    # cross a 64 KB DMA boundary; ES advances by 0x20 (512 bytes) each time.
    mov ax, KERNEL_SEG
    mov es, ax
    mov si, 1                    # LBA of the first kernel sector (LBA 0 = this sector)
    mov di, KERNEL_SECTORS

read_loop:
    mov ax, si
    xor dx, dx
    div word ptr [spt]           # AX = track (LBA / spt), DX = LBA % spt
    mov cl, dl
    inc cl                       # CL bits 0-5 = sector (1-based)
    xor dx, dx
    div word ptr [heads]         # AX = cylinder, DX = head
    mov dh, dl                   # DH = head
    mov ch, al                   # CH = cylinder bits 0-7
    mov bl, ah
    shl bl, 6
    or cl, bl                    # CL bits 6-7 = cylinder bits 8-9
    mov dl, [boot_drive]
    xor bx, bx                   # ES:BX = ES:0000
    mov ax, 0x0201               # AH=02 read, AL=1 sector
    int 0x13
    jc disk_error

    mov ax, es
    add ax, 0x20
    mov es, ax
    inc si
    dec di
    jnz read_loop

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

    jmp 0x08:0x10000

# disk_error runs in REAL mode, so it must be assembled as 16-bit code.
.code16
disk_error:
    mov ax, 0xB800
    mov es, ax
    mov word ptr es:[0], 0x4F45  # red-background white 'E'
.loop:
    cli
    hlt
    jmp .loop

boot_drive: .byte 0
spt:        .word 0
heads:      .word 0

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