OUTPUT_ARCH(arm)

ENTRY(_start)

MEMORY {
    ram : ORIGIN = 0x00102180, LENGTH = 0xDE80
    rom : ORIGIN = 0x00201000, LENGTH = 0x1F000
    dram : ORIGIN = 0x87B00000, LENGTH = 0x500000
}

ramBase = ORIGIN(ram);
romBase = ORIGIN(rom);
dramBase = ORIGIN(dram);

SECTIONS {

    . = romBase;
    .start ALIGN(4) : {	
        *(.text.start)
    } >rom
    
    . = romBase + 0x01FC;
    .rom_info ALIGN(4) : {	    	    	    	     
    	*(.data.rom_info)
    } >rom 

    .text ALIGN(4) : {
        *(.text)
        *(.text.*)        
    } >rom

    .rodata ALIGN(4) : {
        *(.rodata)
        *(.rodata.*)        
    } >rom

    .data ALIGN(4) : {
        *(.data)
        *(.data.*)        
    } >rom

    .got ALIGN(4) : {
        *(.got)
        *(.got.*)        
    } >rom

    __boot_end = .;

    . = ramBase;
    . = ramBase + 0x30;
    .bss ALIGN(16) : {
        _bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        _bss_end = .;
    } >ram

    . = dramBase;
    .dram ALIGN(4) : {
	_dram_start = .;
	*(.dram)
	_dram_end = .;
    } >dram
}
