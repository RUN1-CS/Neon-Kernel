ENTRY(kernel_main)
OUTPUT_FORMAT("binary")   /* raw kernel.bin, no headers */

SECTIONS {
    . = 0x10000;          /* must match the load address in boot.s */

    .text : {
        KEEP(*(.text.entry))     /* kernel_main always comes first, whatever the link order */
        *(.text .text.*)
    }

    .rodata : { *(.rodata .rodata.*) }

    .data : { *(.data .data.*) }

    .bss : {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    }

    /DISCARD/ : { *(.eh_frame) *(.comment) *(.note*) }
}
