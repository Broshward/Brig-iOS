TOP=$(CURDIR)
PROGRAM=main
FILES=main.c startup.c interrupts.c crontab.c strings.c uart.c spi.c adc.c dma.c
LIBDIR=$(TOP)/STM32F10x_StdPeriph_Lib_V3.5.0
#Adust the following line to the library in use
STMLIB=$(LIBDIR)/Libraries
TC=""arm-none-eabi-
CC=$(TC)gcc
LD=$(TC)gcc 
OBJCOPY=$(TC)objcopy
DUMP=$(TC)objdump
AR=$(TC)ar
GDB=$(TC)gdb
SIZE=$(TC)size
STDLIBDIR=/usr/arm-none-eabi/lib
STDINCDIR=/usr/arm-none-eabi/include

INCLUDE=-I$(TOP)
INCLUDE+= -I$(STMLIB)/CMSIS/CM3/CoreSupport
INCLUDE+= -I$(STMLIB)/CMSIS/CM3/DeviceSupport/ST/STM32F10x
INCLUDE+= -I$(STMLIB)/STM32F10x_StdPeriph_Driver/inc
INCLUDE+= -I$(STDINCDIR)
COMMONFLAGS=-O0 -g3 --std=c99 -mcpu=cortex-m3 -mthumb 
CFLAGS=$(COMMONFLAGS) -Wall -Wno-address-of-packed-member $(INCLUDE) 
#Adust the following line to the type of MCU used
CFLAGS+=-D STM32F10X_LD
CFLAGS+=-D USE_STDPERIPH_DRIVER
CFLAGS+=-D VECT_TAB_FLASH
CFLAGS+=-fdata-sections -ffunction-sections
#CFLAGS+=--specs=$(STDLIBDIR)/nosys.specs #-nostdlib

LIBS=-lstm32 -lm -lc -lgcc -lg #-lnosys #-lrdimon

LFLAGS  = -nostartfiles -Wl,-Tlinker.ld -L$(LIBDIR) -L$(STDLIBDIR)
LFLAGS += -fno-exceptions -Wl,-X,--gc-sections,--no-whole-archive
LFLAGS_LIBS = #-fno-exceptions -ffunction-sections -fdata-sections -Wl,-X,--gc-sections

all: clean compile tags disasm size #info

#-----------------------------------------------------------------------------------------------------
clean:
	rm -f $(PROGRAM).lst *.o $(PROGRAM).elf 

link: 
	@ echo "...linking"
	$(LD) $(LFLAGS) -o $(PROGRAM).elf *.o $(LIBS)
	@echo "done."

compile: 
	@ echo "..compiling"
	$(CC) $(CFLAGS) $(LFLAGS) $(FILES) $(LIBS) -o $(PROGRAM).elf 
	@echo "done."

disasm:
	@echo "....disassembler"
	LC_ALL=ru_RU.utf8 $(DUMP) -Shg $(PROGRAM).elf > $(PROGRAM).lst
	
info:
	@echo ".....info"
	$(DUMP) -h $(PROGRAM).elf 

size:
	@echo 'SIZE:'
	$(SIZE) -d $(PROGRAM).elf	

clean_tags:
	rm tags
	
tags: clean_tags
	$(CC) -M $(CFLAGS) $(FILES) | sed -e 's/[\\ ]/\n/g' | \
		sed -e '/^$$/d' -e '/\.o:[ \t]*$$/d' | \
		ctags -L - --c++-kinds=+p --fields=+iaS --extras=+q 

#-----------------------------------------------------------------------------------------------------
libs: clean_libs
	@echo -n ".Building libs"
	@echo -n $(TOP)
	@cd $(STMLIB)/CMSIS/CM3/CoreSupport && \
		$(CC) $(CFLAGS) $(LFLAGS_LIBS) -c core_cm3.c
	@echo
	@echo "CoreSupport"
	@cd $(STMLIB)/CMSIS/CM3/DeviceSupport/ST/STM32F10x && \
		$(CC) $(CFLAGS) $(LFLAGS_LIBS) -c system_stm32f10x.c
	@echo "DeviceSupport"
	@cd $(STMLIB)/STM32F10x_StdPeriph_Driver/src && \
		$(CC) $(CFLAGS) $(LFLAGS_LIBS) -c \
			-D"assert_param(expr)=((void)0)" \
			-I../../CMSIS/CM3/CoreSupport \
			-I../../CMSIS/CM3/DeviceSupport/ST/STM32F10x \
			-I../inc \
			*.c
	@echo "PerifDrivers"
	@$(AR) r $(LIBDIR)/libstm32.a \
		$(STMLIB)/CMSIS/CM3/CoreSupport/core_cm3.o \
		$(STMLIB)/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.o \
		$(STMLIB)/STM32F10x_StdPeriph_Driver/src/*.o
	@echo "done."

clean_libs:
	rm -f $(STMLIB)/STM32F10x_StdPeriph_Driver/src/*.o
	rm -f $(STMLIB)/CMSIS/CM3/CoreSupportccore_cm3.o
	rm -f $(STMLIB)/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.o
	rm -f $(LIBDIR)/libstm32.a

#Compiler optimize settings:
# -O0 no optimize, reduce compilation time and make debugging produce the expected results (default).
# -O1 optimize, reduce code size and execution time, without much increase of compilation time.
# -O2 optimize, reduce code execution time compared to ‘O1’, increase of compilation time.
# -O3 optimize, turns on all optimizations, further increase of compilation time.
# -Os optimize for size, enables all ‘-O2’ optimizations that do not typically increase code size and other code size optimizations.
# default settings for release version: COMMONFLAGS=-O3 -g -mcpu=cortex-m3 -mthumb
# default settings for debug version: COMMONFLAGS=-O0 -g -mcpu=cortex-m3 -mthumb

#---------------------------------------------------------------------------------------

