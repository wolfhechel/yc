
ifeq ($(OS),Windows_NT)
DIR = C:/gcc-arm-none-eabi-7-2018-q2-update-win32
else
DIR = /usr/gcc-arm-none-eabi-7-2018-q2-update
endif

ARCH = $(DIR)/bin/arm-none-eabi
GCC = $(ARCH)-gcc
LD = $(ARCH)-ld
AS = $(ARCH)-as
OBJCOPY = $(ARCH)-objcopy
OBJDUMP = "$(ARCH)-objdump"
LIB_PATH = ../../../../../Librarier
USER_PATH = ../../user


all : output/out.hex


CORE_OBJS =  $(LIB_PATH)/startup/flash_start_gcc \
             $(LIB_PATH)/core/system \
             $(LIB_PATH)/core/misc \
             $(LIB_PATH)/sdk/yc_gpio \
             $(LIB_PATH)/sdk/yc_uart \
             $(LIB_PATH)/sdk/yc_timer

             
USER_OBJS = $(USER_PATH)/main
            


OBJS = $(CORE_OBJS) $(USER_OBJS)

INC_DIR = -I $(LIB_PATH)/core \
          -I $(LIB_PATH)/sdk


CFLAG =  -mthumb -mcpu=cortex-m0 -c -g -Os $(INC_DIR) $(DEF) -fno-toplevel-reorder -ffunction-sections -fdata-sections
LDFLAG = -T output/ld.script -Map=output/memmap -lc -lm -lgcc -L "$(DIR)/lib/gcc/arm-none-eabi/7.3.1/thumb/v6-m"  -L "$(DIR)/arm-none-eabi/lib/thumb/v6-m"

define ldscript = 
/* Linker script to configure memory regions. 
 * Need modifying for a specific board. 
 *   FLASH.ORIGIN: starting address of flash
 *   FLASH.LENGTH: length of flash
 *   RAM.ORIGIN: starting address of RAM bank 0
 *   RAM.LENGTH: length of RAM bank 0
 */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x1000000, LENGTH = 0x80000 /* 512K */
  RAM (rwx) : ORIGIN = 0x20000, LENGTH = 0x10000 /* 64K */
}

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions FLASH and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 * 
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __copy_table_start__
 *   __copy_table_end__
 *   __zero_table_start__
 *   __zero_table_end__
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 */
ENTRY(Reset_Handler)

SECTIONS
{
	.text :
	{
		*flash_start*.o
		*(.text*)

		KEEP(*(.init))
		KEEP(*(.fini))

		/* .ctors */
		*crtbegin.o(.ctors)
		*crtbegin?.o(.ctors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
		*(SORT(.ctors.*))
		*(.ctors)

		/* .dtors */
 		*crtbegin.o(.dtors)
 		*crtbegin?.o(.dtors)
 		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
 		*(SORT(.dtors.*))
 		*(.dtors)

		*(.rodata*)

		KEEP(*(.eh_frame*))
	} > FLASH
	
	

	.ARM.extab : 
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > FLASH

	
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > FLASH

	 . = ALIGN(4);
	__exidx_start = .;

	__etext = .;
		
	.data : AT (__etext)
	{
		__data_start__ = .;
		*(vtable)
		*(.data*)

		. = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);

		. = ALIGN(4);
		/* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);


		. = ALIGN(4);
		/* finit data */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);

		KEEP(*(.jcr*))
		. = ALIGN(4);
		/* All data end */
		__data_end__ = .;

	} > RAM

	.bss :
	{
		. = ALIGN(4);
		__bss_start__ = .;
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;
	} > RAM
	
	.heap (COPY):
	{
		__end__ = .;
		PROVIDE(end = .);
		*(.heap*)
		__HeapLimit = .;
	} > RAM

	/* .stack_dummy section doesn't contains any symbols. It is only
	 * used for linker to calculate size of stack sections, and assign
	 * values to stack symbols later */
	.stack_dummy (COPY):
	{
		*(.stack*)
	} > RAM

	/* Set stack top to end of RAM, and stack limit move down by
	 * size of stack_dummy section */
	__StackTop = ORIGIN(RAM) + LENGTH(RAM);
	__StackLimit = __StackTop - SIZEOF(.stack_dummy);
	PROVIDE(__stack = __StackTop);
	
	/* Check if data + heap + stack exceeds RAM limit */
	ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
}
endef


output/out.hex : output/out
	$(OBJCOPY) $^ -O ihex $@


output/out : $(addprefix output/, $(addsuffix .o, $(OBJS)))
	$(file >output/ld.script, $(ldscript))
	$(LD) $(addprefix output/obj/,$(addsuffix .o, $(notdir $(OBJS)))) -o $@ $(LDFLAG)
	$(OBJDUMP) -S $@ > output/disasm

output/%.o : %.c
	$(GCC) $< $(CFLAG) -o output/obj/$(notdir $@)

output/%.o : %.s
	$(AS) $^ -mthumb -mcpu=cortex-m0 -o output/obj/$(notdir $@)


clean :
ifeq ($(OS),Windows_NT)
	del /Q output\*.*
	del /Q output\obj\*.*
else
	rm -rf output/*
	rm -rf output/obj/*
endif
