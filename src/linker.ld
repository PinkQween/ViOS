ENTRY(_start)
/* OUTPUT_FORMAT(binary) */
SECTIONS
{
    . = 1M;
    .text : ALIGN(4096)
    {
        __text_start = .;
        *(.text)
        __text_end = .;
    }

    .asm : ALIGN(4096)
    {
        __asm_start = .;
        *(.asm)
        __asm_end = .;
    }

    .rodata : ALIGN(4096)
    {
        __rodata_start = .;
        *(.rodata)
        __rodata_end = .;
    }

    .data : ALIGN(4096)
    {
        __data_start = .;
        *(.data)
        __data_end = .;
    }

    .bss : ALIGN(4096)
    {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        __bss_end = .;
    }
}