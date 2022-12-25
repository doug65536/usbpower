.SUFFIXES:

define gcc_from_arch=
$(2)CC = $(1)gcc
$(2)CXX = $(1)g++
$(2)LD = $(1)ld
$(2)RANLIB = $(1)ranlib
$(2)AS = $(1)as
$(2)AR = $(1)ar
$(2)SIZE = $(1)size
$(2)CPP = $(1)cpp
$(2)GDB = $(1)gdb
$(2)NM = $(1)nm
$(2)OBJCOPY = $(1)objcopy
$(2)OBJDUMP = $(1)objdump
$(2)NM = $(1)nm
$(2)STRIP = $(1)strip
$(2)GPROF = $(1)gprof
$(2)GCOV = $(1)gcov
endef

$(eval $(call gcc_from_arch,,BUILD_))
$(eval $(call gcc_from_arch,avr-,AVRC_))
$(eval $(call gcc_from_arch,arm-none-eabi-,M0C_))

M0C_CXXFLAGS += -I$(SRC_DIR)/arch/m0 -mcpu=cortex-m0 -fno-exceptions
M0C_CXXFLAGS += -ffreestanding -fbuiltin -nostdlib -g -Os
#M0C_CXXFLAGS += -flto

M0C_LIBGCC = $(shell $(M0C_CXX) $(M0C_CXXFLAGS) -print-libgcc-file-name)
$(info libgcc is $(M0C_LIBGCC))

SRCDIR ?= .
AVR_MCU = atmega32u4
AVR_MCU_FREQ = 16000000
AVRDUDE = avrdude
AVR_QEMU = qemu-system-avr
M0_QEMU = qemu-system-arm
QEMU_AVR_MCU = atmega328p
QEMU_MACHINE = uno
LESS = less
MKDIR = mkdir
RM = rm
GDB_MULTIARCH = gdb-multiarch
SRC_DIR = .

LANGUAGEFLAGS = -W -Wall -Wextra -Werror=return-type \
	-Werror -Wno-error=unused-function -Wno-error=unused-variable \
	-Wno-error=unused-parameter -std=c++17

# BUILD_ named things are for things that compile and run on the build machine
# For generators that run on the build machine
BUILD_CC = gcc
BUILD_CXX = g++
BUILD_CXXFLAGS = $(BUILD_COMPILEFLAGS) $(LANGUAGEFLAGS) -g
OUTNAMEBASE = usbtoy_blinky
OUTNAME = $(OUTNAMEBASE)-$(AVR_MCU)
QEMU_OUTNAME = $(OUTNAMEBASE)-$(QEMU_AVR_MCU)

IO_OUTNAME = $(OUTNAMEBASE)-io

all: $(OUTNAME).hex $(IO_OUTNAME).bin genfont

clean:
	$(RM) -f $(OUTNAME).hex $(QEMU_OUTNAME).elf $(OUTNAME).elf \
		$(COMBINED_OBJECTS_ALL) \
		genfont 

.PHONY: all clean flash hex fuses \
	disassemble-avr disassemble-m0 \
	qemu-debug-avr qemu-debug-m0 debug-avr debug-m0

AVRC_COMPILEFLAGS = -mmcu=$(AVR_MCU) -g # -flto
#CXXFLAGS = $(COMPILEFLAGS) -O0 -std=c++17 -DF_CPU=$(MCU_FREQ) 
AVRC_CXXFLAGS = $(AVRC_COMPILEFLAGS) $(LANGUAGEFLAGS)
AVRC_CXXFLAGS += -DF_CPU=$(AVR_MCU_FREQ)
AVRC_CXXFLAGS += -Os -flto
AVRC_CXXFLAGS += -I$(SRC_DIR)/arch/avr
#AVRC_CXXFLAGS += -O0

BUILD_CXXFLAGS = $(BUILD_COMPILEFLAGS) $(BUILD_LANGUAGEFLAGS)
BUILD_CXXFLAGS += -g -Os

AVRC_SRCS = main.cc clk.cc ctx_init_avr.cc ctx_avr.S task.cc timer.cc \
	usb.cc debug.cc render.cc display-ST7735R-bitbang.S

IO_SRCS = io_main.cc task.cc timer.cc ctx_init_m0.cc ctx_m0.S debug.cc \
	arch/m0/reset.S

GENFONT_SRCS = genfont.cc

BUILD_SRCS = $(GENFONT_SRCS)

include autodep.mk

$(eval $(call extract_names,AVRC,AVRC,$(AVR_MCU),0))
$(eval $(call compile_targets,AVRC,AVRC))

$(eval $(call extract_names,BUILD,BUILD,host,1))
$(eval $(call compile_targets,BUILD,BUILD))

$(eval $(call extract_names,IO,M0C,m0,1))
$(eval $(call compile_targets,IO,M0C))

# Font data generator

$(eval $(call extract_names,GENFONT,BUILD,host,1))

genfont: $(GENFONT_OBJECTS_ALL)
	$(BUILD_CXX) $(BUILD_CXXFLAGS) -o $@ $^ -lpng

FONT_NAMES = liberation_sans_narrow_bold_48 \
	liberation_sans_narrow_regular_16

FONT_SRCS = $(patsubst %,%.cc,$(FONT_NAMES))

$(eval $(call extract_names,FONT,FONT,$(AVR_MCU),0))

# $(info Adding rule to generate $$(AVRC_OBJDIR)/$(1).cc \
# 	from $$(SRCDIR)/textimg/$(1).sfl)
define generate_font=
$$(AVRC_OBJDIR)/$(1).cc: $$(SRCDIR)/textimg/$(1).sfl genfont
	$$(MKDIR) -p $$(@D)
	./genfont $$< > $$@
endef

$(eval $(call combine_objects))

$(foreach file,$(FONT_NAMES), \
	$(eval $(call generate_font,$(file))))

# $(foreach file,$(FONT_SOURCE_NAMES_CC), \
# 	$(info font source name cc: $(file)))

FONT_CXXFLAGS = $(AVRC_CXXFLAGS) -I$(abspath $(SRC_DIR))

$(foreach file,$(FONT_SOURCE_NAMES_CC), \
	$(eval $(call compile_extension,$(file),cc,AVRC_CXX,FONT_CXXFLAGS,AVRC_OBJDIR,AVRC_OBJDIR)))

$(IO_OUTNAME).elf: $(IO_OBJECTS_ALL) $(SRC_DIR)/arch/m0/fwlink.ld
	$(M0C_CXX) $(M0C_CXXFLAGS) \
		-Wl,-T,$(SRC_DIR)/arch/m0/fwlink.ld \
		-Wl,-Map,$@.map \
		-o $@ $(IO_OBJECTS_ALL) $(M0C_LIBGCC)

$(IO_OUTNAME).bin: $(IO_OUTNAME).elf
	$(M0C_OBJCOPY) --strip-debug -Obinary $< $@

$(OUTNAME).elf: $(OBJECTS_ALL) $(FONT_OBJECTS_ALL)
	$(AVRC_CXX) $(AVRC_CXXFLAGS) -Wl,-Map,$@.map -o $@ $^
	$(AVRC_SIZE) --format=avr --mcu=$(AVR_MCU) $@

$(OUTNAME).hex: $(OUTNAME).elf
	$(AVRC_OBJCOPY) -j .text -j .data -O ihex $< $@

hex: $(OUTNAME).hex

flash: $(OUTNAME).hex
	$(AVRDUDE) -c flip1 -p $(AVR_MCU) -U flash:w:$(OUTNAME).hex:i

fuses:
	$(AVRDUDE) -c flip1 -p $(AVR_MCU) -U lfuse:w:0x40:m -u

disassemble-avr: $(OUTNAME).elf
	$(AVRC_OBJDUMP) -S $^

qemu-disassemble-m0: $(IO_OUTNAME).elf
	$(M0C_OBJDUMP) -S $^

qemu-debug-avr: $(QEMU_OUTNAME).elf
	$(AVR_QEMU) -M $(QEMU_MACHINE) -bios $^ -s -S

qemu-debug-m0: $(IO_OUTNAME).elf
	$(M0_QEMU) -M microbit -kernel $^ -s -S

debug-avr: $(QEMU_OUTNAME).elf
	$(AVRC_GDB) $^ -iex 'target extended-remote :1234' \
		$(GDBFLAGS) \
		-ex 'b __vector_21' \
		-ex 'b __vector_13' \
		-ex 'b __vector_11' \
		-ex 'b __vector_16'

#-iex 'set architecture armv6-m'

debug-m0: $(IO_OUTNAME).elf
	$(GDB_MULTIARCH) \
		-iex 'file $^' \
		-iex 'target extended-remote :1234' \
		-iex 'b __unhandled_exception'
		-iex 'layout src'
		$(GDBFLAGS)

-include $(DEPFILES)
