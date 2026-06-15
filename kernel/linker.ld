ENTRY(kernel_main)
OUTPUT_FORMAT("binary") /* Forces the linker to output raw kernel.bin, no metadata */

SECTIONS {
    . = 0x1000;         /* Tell compiler the kernel will be loaded at address 0x1000 */

    .text : { *(.text) }
    .data : { *(.data) }
    .bss  : { *(.bss) }
}