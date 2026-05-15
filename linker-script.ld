ENTRY(reset_handler) /* define start as the entry address */
SECTIONS /* program sections */
{
    . = 0x10000; /* loading address, required by QEMU */
    .text : { *(.text) } /* all text in .text section */
    .data : { *(.data) } /* all data in .data section */
    .bss : { *(.bss) } /* all bss in .bss section */
    .data : {image0.o}
    . = ALIGN(8);
    . = . + 0x1000; /* 4KB stack space */
    svc_stack_top = .;
    . = . + 0x1000; /* 4kB of IRQ stack space */
    irq_stack_top = .;
}