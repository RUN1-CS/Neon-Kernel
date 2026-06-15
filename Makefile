# Toolchain definition
CC = gcc
LD = ld
ASM = nasm

CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector -O2 -Wall -Wextra
ASMFLAGS_BIN = -f bin
ASMFLAGS_ELF = -f elf32

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

# ONLY put auxiliary kernel assembly files here (NOT the main bootloader)
KERNEL_ASM_SOURCES = kernel/drivers/power.asm
KERNEL_ASM_OBJECTS = $(KERNEL_ASM_SOURCES:.asm=.o)

all: $(OS_IMAGE)

# Step 4C: Stitch bootloader and kernel into the final OS Image
$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	cat $(BOOT_BIN) $(KERNEL_BIN) > $(OS_IMAGE)
	truncate -s 1440k $(OS_IMAGE)

# Step 4A: Assemble the 512-byte master bootloader to flat binary
$(BOOT_BIN): boot/boot.asm
	$(ASM) $(ASMFLAGS_BIN) $< -o $@

# Rule to assemble kernel assembly files into ELF32 object files
kernel/drivers/%.o: kernel/drivers/%.asm
	$(ASM) $(ASMFLAGS_ELF) $< -o $@

# Step 4B: Link all C and ASM object files into flat kernel.bin using the Linker Script
$(KERNEL_BIN): $(KERNEL_OBJECTS) $(KERNEL_ASM_OBJECTS)
	$(LD) -m elf_i386 -T kernel/linker.ld -o $@ $(KERNEL_OBJECTS) $(KERNEL_ASM_OBJECTS)

# Compile C files to bare-metal object files
kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Fallback pattern for files inside apps/xxsh/
apps/%.o: apps/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(OS_IMAGE)
	qemu-system-i386 -fda $(OS_IMAGE)

clean:
	rm -f $(BOOT_BIN) $(KERNEL_BIN) $(KERNEL_OBJECTS) $(KERNEL_ASM_OBJECTS) $(OS_IMAGE)
	find . -name "*.o" -type f -delete