# Toolchain definition
CC = gcc
LD = ld
ASM = nasm

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -Wall -Wextra
ASMFLAGS = -f bin

# Target Outputs
BOOT_BIN = boot/boot.bin
KERNEL_BIN = kernel/kernel.bin
OS_IMAGE = SSn-DOS.img

# Source Files
KERNEL_SOURCES = kernel/main.c \
                 kernel/drivers/video.c \
                 kernel/drivers/keyboard.c \
				 kernel/drivers/string.c \
                 apps/xxsh/shell.c \
                 apps/xxsh/executor.c
KERNEL_OBJECTS = $(KERNEL_SOURCES:.c=.o)

all: $(OS_IMAGE)

# Step 4C: Stitch bootloader and kernel into the final OS Image
$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	# Stitch the components together
	cat $(BOOT_BIN) $(KERNEL_BIN) > $(OS_IMAGE)
	# Pad the image file with zeroes until it is exactly 1.44MB
	truncate -s 1440k $(OS_IMAGE)

# Step 4A: Assemble the 512-byte bootloader
$(BOOT_BIN): boot/boot.asm
	$(ASM) $(ASMFLAGS) $< -o $@

# Step 4B: Link object files into flat kernel.bin using the Linker Script
$(KERNEL_BIN): $(KERNEL_OBJECTS)
	$(LD) -m elf_i386 -T kernel/linker.ld -o $@ $^

# Compile C files to bare-metal object files
kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(OS_IMAGE)
	qemu-system-i386 -fda $(OS_IMAGE)

clean:
	rm -f $(BOOT_BIN) $(KERNEL_BIN) $(KERNEL_OBJECTS) $(OS_IMAGE)