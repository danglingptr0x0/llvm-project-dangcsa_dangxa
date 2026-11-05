ENTRY(_rst)

PROVIDE(__stack_top = 0x00010000);
PROVIDE(__argc = 1);

SECTIONS
{
    . = 0x00000000;

    .text : {
        KEEP(*(.text._reset))
        KEEP(*(.text._start))
        *(.text .text.*)
    }

    .rodata : ALIGN(8) {
        *(.rodata .rodata.*)
        *(.srodata .srodata.*)
    }

    . = ALIGN(8);
    .preinit_array : ALIGN(8) {
        __preinit_array_start = .;
        KEEP(*(.preinit_array))
        __preinit_array_end = .;
    }

    . = ALIGN(8);
    .init_array : ALIGN(8) {
        __init_array_start = .;
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
        KEEP(*(.init_array))
        KEEP(*(.ctors))
        __init_array_end = .;
    }

    . = ALIGN(8);
    .fini_array : ALIGN(8) {
        __fini_array_start = .;
        KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)))
        KEEP(*(.fini_array))
        KEEP(*(.dtors))
        __fini_array_end = .;
    }

    .data : ALIGN(8) {
        *(.data .data.*)
        *(.sdata .sdata.*)
    }

    . = ALIGN(8);
    .rela.dyn : ALIGN(8) {
        __rel_start = .;
        *(.rela.dyn)
        *(.rela.*)
        *(.rel.dyn)
        *(.rel.*)
        __rel_end = .;
    }

    . = ALIGN(8);
    __bss_start = .;

    .bss : ALIGN(8) {
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        *(COMMON)
    }

    . = ALIGN(8);
    __bss_end = .;
    __heap_start = .;

    /DISCARD/ : {
        *(.comment)
        *(.note .note.*)
        *(.eh_frame .eh_frame_hdr)
        *(.debug_gdb_scripts)
    }
}
